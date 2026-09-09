#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gl_utils.h"

#include "imgui/imgui.h"

#include "tiny_expr/tinyexpr.h"

#include "cube.h"
#include "logging.h"
#include "mesh.h"
#include "mesh_bounds.h"
#include "mesh_gpu.h"
#include "mesh_io.h"
#include "navier_stokes.h"
#include "ndc.h"
#include "shaders.h"
#include "sphere.h"
#include "viewer.h"

// Viewer config
float bgcolor[4] = {0.3, 0.3, 0.3, 1.0};
bool draw_surface = true;
bool draw_edges = false;
float scale_min;
float scale_max;
float mesh_deform = 0;

// FEM interaction
bool autoscale = true;
bool started = false;
bool one_step = false;
bool reset = false;

// Parameters
float lognu = -3.7;
float dt = 0.002;
double tol = 1e-6;

// RHS expression of the PDE
char rhs_expression[128] = "100 * z * exp(-50*z^2) * (1 + 0.5 * cos(20 * theta))";
bool rhs_show_error = false;
double rhs_x, rhs_y, rhs_z, rhs_p, rhs_t, rhs_r;
te_variable rhs_vars[] = {{"x", &rhs_x},   {"y", &rhs_y},     {"z", &rhs_z},
			  {"phi", &rhs_p}, {"theta", &rhs_t}, {"rand", &rhs_r}};
te_expr *te_rhs = NULL;


// Forward declarations
static void rescale_and_recenter_mesh(Mesh & mesh);
static void init_camera_for_mesh(const Mesh & mesh, Camera & camera);
static void update_all(NavierStokesSolver & solver, Mesh & mesh, GPUMesh & mesh_gpu);
static void draw_scene(const Viewer & viewer, GLuint shader, const GPUMesh & gpu_mesh);
static void draw_gui(NavierStokesSolver & solver);
static void key_cb(int key, int action, int mods, void *args);
static void get_attr_bounds(const Mesh & m, float *attr_min, float *attr_max);


/*
 * Initialize the Navier-Stokes solver with the initial vorticity
 * defined by the mathematical expression entered by the user.
 *
 * The expression is evaluated at every mesh vertex using its
 * spherical coordinates. The vorticity is then centered to have
 * zero mean and the stream function is reset.
 */
void reset_solver(NavierStokesSolver & solver) {
	for (size_t i = 0; i < solver.N; ++i) {
		rhs_x = solver.m.positions[i].x;
		rhs_y = solver.m.positions[i].y;
		rhs_z = solver.m.positions[i].z;

		// Compute spherical coordinates used by the RHS expression
		rhs_p = atan2(rhs_y, rhs_x);
		rhs_t = atan2(sqrt(rhs_x * rhs_x + rhs_y * rhs_y), rhs_z);

		// Random value available to the user-defined expression
		rhs_r = (double)rand() / RAND_MAX;

		// Evaluate the initial vorticity at this mesh vertex
		solver.omega[i] = te_eval(te_rhs);
	}

	// Navier-Stokes formulation requires a zero-mean vorticity
	solver.set_zero_mean(solver.omega.data);

	// Reset the stream function and simulation time
	memset(solver.psi.data, 0, solver.N * sizeof(double));
	solver.t = 0;
}


/*
 * Compile the mathematical expression defining the initial
 * vorticity and apply it to the solver.
 *
 * If the expression is invalid, the previous expression is kept.
 */
bool new_rhs(NavierStokesSolver & solver) {
	srand((int)time(NULL));

	// Compile the expression entered in the GUI
	te_expr *test =
	    te_compile(rhs_expression, rhs_vars,
		       sizeof(rhs_vars) / sizeof(rhs_vars[0]), NULL);

	if (!test)
		return false;

	// Replace the previous compiled expression
	te_free(te_rhs);
	te_rhs = test;

	reset_solver(solver);

	return true;
}


/*
 * Copy a scalar FEM field from the solver to the mesh.
 *
 * The values stored in the solver are copied into mesh.attr
 * so that they can later be uploaded to the GPU and displayed
 * using the scalar visualization shader.
 */
void transfer_to_mesh(const TArray<double> & V, Mesh & m) {
	m.attr.resize(m.vertex_count());

	for (size_t i = 0; i < m.vertex_count(); ++i) {
		m.attr[i] = V[i];
	}
}


/*
 * Rescale and recenter the mesh so that it fits inside a
 * normalized coordinate system centered around the origin.
 *
 * This makes the mesh easier to display with the viewer and
 * gives the camera a predictable scale.
 */
static void rescale_and_recenter_mesh(Mesh & mesh) {

	Aabb bbox = compute_mesh_bounds(mesh);

	// Compute the center and largest dimension of the mesh
	Vec3 model_center = (bbox.min + bbox.max) * 0.5f;
	Vec3 model_extent = (bbox.max - bbox.min);
	float model_size = max(model_extent);

	if (model_size == 0) {
		printf("Warning : Mesh is empty or reduced to a point.\n");
		model_size = 1;
	}

	// Translate the mesh to the origin and normalize its size
	for (size_t i = 0; i < mesh.vertex_count(); ++i) {
		mesh.positions[i] -= model_center;
		mesh.positions[i] /= (model_size / 2);
	}
}


/*
 * Initialize the camera according to the mesh bounding box.
 *
 * The camera is positioned along the positive Z direction,
 * looking toward the center of the mesh. Near and far planes
 * are also scaled according to the mesh size.
 */
static void init_camera_for_mesh(const Mesh & mesh, Camera & camera) {

	Aabb bbox = compute_mesh_bounds(mesh);

	Vec3 model_center = (bbox.min + bbox.max) * 0.5f;
	Vec3 model_extent = (bbox.max - bbox.min);
	float model_size = max(model_extent);

	if (model_size == 0) {
		printf("Warning : Mesh is empty or reduced to a point.\n");
		model_size = 1;
	}

	camera.set_target(model_center);

	Vec3 start_pos =
	    (model_center + 2.f * Vec3(0, 0, model_size));

	camera.set_position(start_pos);
	camera.set_near(0.01 * model_size);
	camera.set_far(100 * model_size);
}


/*
 * Compute the minimum and maximum values of the scalar field
 * stored in mesh.attr.
 *
 * These bounds are used by the fragment shader to map scalar
 * values to colors.
 */
static void get_attr_bounds(const Mesh & m, float *attr_min, float *attr_max) {

	if (!m.vertex_count())
		return;

	float min = m.attr[0];
	float max = min;

	for (size_t i = 1; i < m.vertex_count(); ++i) {
		if (m.attr[i] < min) {
			min = m.attr[i];
		} else if (m.attr[i] > max) {
			max = m.attr[i];
		}
	}

	*attr_min = min;
	*attr_max = max;
}


/*
 * Update the Navier-Stokes simulation and synchronize the
 * resulting vorticity field with the GPU mesh.
 *
 * A time step is performed when the simulation is running
 * or when the user requests a single step. The scalar bounds
 * are recomputed when autoscaling is enabled.
 */
static void update_all(NavierStokesSolver & solver, Mesh & mesh, GPUMesh & gpu_mesh) {
    
    bool needs_upload = true;

	if (started || one_step) {
		// Advance the Navier-Stokes solver by one time step
		solver.time_step(dt, pow(10, lognu));

		// A single-step request is consumed after one iteration
		if (one_step) {
			one_step = false;
		}

		// Copy the updated vorticity to the visualization mesh
		transfer_to_mesh(solver.omega, mesh);

		// Update the color scale according to the current field
		if (autoscale) {
			get_attr_bounds(mesh, &scale_min, &scale_max);
		}
	} else if (reset) {
		// Restore the initial vorticity and reset the simulation
		reset_solver(solver);
		transfer_to_mesh(solver.omega, mesh);
		get_attr_bounds(mesh, &scale_min, &scale_max);
		reset = false;
	} else {
		// Nothing changed, so there is no need to update the GPU
		needs_upload = false;
	}

	if (needs_upload) {
		gpu_mesh.update_attr();
	}
}


/*
 * Render the current scalar field stored in the GPU mesh.
 *
 * The camera matrices and visualization parameters are sent
 * to the shader. The mesh can be rendered either as a filled
 * surface, as edges, or both.
 */
static void draw_scene(const Viewer & viewer, GLuint shader, const GPUMesh & gpu_mesh) {

	glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const Camera &camera = viewer.camera;

	glUseProgram(shader);

	// Compute the matrices required by the vertex shader
	Mat4 proj = camera.view_to_clip();
	Mat4 vm = camera.world_to_view();
	Vec3 camera_pos = camera.get_position();

	// Locate shader uniforms
	GLint vm_loc = glGetUniformLocation(shader, "vm");
	GLint proj_loc = glGetUniformLocation(shader, "proj");
	GLint camera_pos_loc = glGetUniformLocation(shader, "camera_pos");
	GLint scale_min_loc = glGetUniformLocation(shader, "scale_min");
	GLint scale_max_loc = glGetUniformLocation(shader, "scale_max");
	GLint deform_loc = glGetUniformLocation(shader, "deform");
	GLint lighting_loc = glGetUniformLocation(shader, "lighting");

	// Send camera and visualization parameters to the shader
	glUniformMatrix4fv(vm_loc, 1, GL_FALSE, &vm(0, 0));
	glUniformMatrix4fv(proj_loc, 1, GL_FALSE, &proj(0, 0));
	glUniform3fv(camera_pos_loc, 1, &camera_pos[0]);
	glUniform1f(scale_min_loc, scale_min);
	glUniform1f(scale_max_loc, scale_max);
	glUniform1f(deform_loc, mesh_deform);

	if (draw_surface) {
		glEnable(GL_POLYGON_OFFSET_FILL);

		float offset = reversed_z ? -1.f : 1.f;
		glPolygonOffset(offset, offset);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Enable lighting for the surface rendering
		glUniform1i(lighting_loc, GL_TRUE);

		gpu_mesh.draw();
	}

	if (draw_edges) {
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.f, 0.f);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// Disable lighting so that mesh edges keep a uniform color
		glUniform1i(lighting_loc, GL_FALSE);

		gpu_mesh.draw();
	}

	// Restore the default polygon mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_BLEND);
}


/*
 * Draw the ImGui control panel used to interact with the
 * Navier-Stokes simulation.
 *
 * The GUI allows the user to modify the initial vorticity,
 * start/stop the simulation, perform one step, reset the
 * solution and change numerical and visualization parameters.
 */
static void draw_gui(NavierStokesSolver & solver) {

	ImGui::Begin("Controls");

	ImGui::Text("Navier Stokes solver");
	ImGui::Text("--------------------");

	ImGui::Text("Enter math expression for initial vorticity below:");
	ImGui::Text("(available variables : x, y, z, phi, theta, rand)");
	ImGui::Text("(zero mean automatically achieved by adding constant)");

	ImGui::InputText("", rhs_expression, IM_ARRAYSIZE(rhs_expression));

	if (ImGui::Button("Apply")) {
		if (!new_rhs(solver)) {
			rhs_show_error = true;
		}

		started = false;
		reset = true;
	}

	if (rhs_show_error) {
		ImGui::Begin("Error");

		ImGui::Text("Syntax error in expresion (missing * ?)");

		if (ImGui::Button("Got it!")) {
			rhs_show_error = false;
		}

		ImGui::End();
	}

	ImGui::Text(" ");

	ImGui::Text("Solution value is represented by color :");
	ImGui::Text("Red = low value, Green = mid, Blue = high.");

	ImGui::Text(" ");

	// Simulation controls
	if (ImGui::Button("Start")) {
		started = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Stop")) {
		started = false;
	}

	ImGui::SameLine();

	if (ImGui::Button("One step")) {
		if (!started) {
			one_step = true;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Reset")) {
		reset = true;
	}

	ImGui::Text("Time : %f", solver.t);

	ImGui::Text("Scale min %.2f Scale max %.2f  (Span : %g)",
		    scale_min,
		    scale_max,
		    scale_max - scale_min);

	ImGui::Text(" ");

	ImGui::Text("Controls");
	ImGui::Text("--------");

	ImGui::Text("Viscosity (negative power of 10):");
	ImGui::SliderFloat("nu", &lognu, -8, 0, "10^(%.1f)");

	ImGui::Text("Time step :");
	ImGui::SliderFloat("dt", &dt, 0.f, 0.01f, "%.4f");

	ImGui::Checkbox("Autoscale colors to bounds", &autoscale);
	ImGui::Checkbox("Show mesh edges", &draw_edges);

	ImGui::Text("Artificially deform mesh according to omega :");
	ImGui::Text("(may help visualize oscillations)");

	ImGui::SliderFloat("  ", &mesh_deform, 0.f, 1.f);

	ImGui::Text(" ");

	ImGui::Text("Number of DOF : %zu", solver.N);

	float fps = ImGui::GetIO().Framerate;
	ImGui::Text("Average framerate : %.1f FPS", fps);

	ImGui::Text(" ");

	ImGui::Text("Mouse :");
	ImGui::Text("Click + drag : orbit");
	ImGui::Text("Click + CTRL + drag : zoom in/out");
	ImGui::Text("Click + SHIFT + drag : translate");

	ImGui::End();
}


/*
 * Handle keyboard shortcuts used by the viewer.
 *
 * S toggles the surface rendering and E toggles the
 * visualization of the mesh edges.
 */
static void key_cb(int key, int action, int mods, void *args) {
	(void)mods;
	(void)args;

	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		draw_surface = !draw_surface;
		return;
	}

	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		draw_edges = !draw_edges;
		return;
	}
}


/*
#######################################################################################################
#######################################################################################################
 * Main entry point of the Navier-Stokes visualization demo.
 * The function loads and normalizes the sphere mesh, initializes
 * the FEM solver and its initial vorticity, creates the OpenGL
 * viewer and GPU resources, then continuously updates and
 * renders the simulation.
#######################################################################################################
#######################################################################################################
 */
int main() {
	log_init(0);

	// Load the sphere mesh used by the Navier-Stokes solver
	Mesh mesh;

	if (load_sphere(mesh, 20)) {
		LOG_MSG("Error loading sphere mesh.");
		exit(EXIT_FAILURE);
	}

	LOG_MSG("Loaded mesh.");

	// Normalize the geometry for visualization
	rescale_and_recenter_mesh(mesh);

	LOG_MSG("Mesh rescaled and recentered.");


	// Prepare FEM data
	NavierStokesSolver solver(mesh);

	if (!new_rhs(solver)) {
		LOG_MSG("Error loading rhs (expression flawed ?).");
		exit(EXIT_FAILURE);
	}

	// Copy the initial vorticity to the mesh for visualization
	transfer_to_mesh(solver.omega, mesh);

	// Compute the initial color range
	get_attr_bounds(mesh, &scale_min, &scale_max);

	LOG_MSG("Prepared FEM data.");


	// Create the OpenGL viewer
	Viewer viewer;

	init_camera_for_mesh(mesh, viewer.camera);

	viewer.init("Navier Stokes 2D solver (vorticity formulation)");

	viewer.register_key_callback({key_cb, NULL});
	viewer.mouse.set_double_click_time(-1);

	// Load the OpenGL functions required by the application
	if (!init_gl()) {
		LOG_MSG("Error initializing OpenGL functions.");
		exit(EXIT_FAILURE);
	}

	LOG_MSG("Viewer initialized.");


	// Compile the shaders used to visualize the scalar field
	GLuint shader = create_shader("shaders/fem.vert", "shaders/fem.frag");

	if (!shader) {
		exit(EXIT_FAILURE);
	}

	LOG_MSG("Shader initialized.");


	// Upload the mesh and its scalar attributes to the GPU
	GPUMesh gpu_mesh;
	gpu_mesh.m = &mesh;
	gpu_mesh.upload();


	// Main rendering and simulation loop
	while (!viewer.should_close()) {
		viewer.poll_events();

		viewer.begin_frame();

		// Draw the user interface first so that button actions
		// are processed before the simulation update
		draw_gui(solver);

		// Update the numerical solution if necessary
		update_all(solver, mesh, gpu_mesh);

		// Render the updated solution
		draw_scene(viewer, shader, gpu_mesh);

		viewer.end_frame();
	}


	// Release viewer resources before exiting
	viewer.fini();

	log_fini();

	return (EXIT_SUCCESS);
}