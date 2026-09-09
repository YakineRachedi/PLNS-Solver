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
#include "ndc.h"
#include "poisson.h"
#include "shaders.h"
#include "sphere.h"
#include "viewer.h"

// Viewer config
float bgcolor[4] = { 0.3, 0.3, 0.3, 1.0 };
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
int iter_per_frame = 1;

// RHS expression of the PDE
// The expression can be modified directly from the ImGui interface
char rhs_expression[128] = "cos(35 * y * sin(27 + 13 * x^2 + 19 * z^2 - 13 * x * z))";
bool rhs_show_error = false;

// Variables used by tinyexpr when evaluating the RHS at each mesh vertex
double rhs_x, rhs_y, rhs_z, rhs_p, rhs_t, rhs_r;

te_variable rhs_vars[] = { { "x", &rhs_x },
							{ "y", &rhs_y },
							{ "z", &rhs_z },
							{ "phi", &rhs_p },
							{ "theta", &rhs_t },
							{ "rand", &rhs_r } };
te_expr *te_rhs = NULL;

// Functions used by the viewer and the simulation
static void rescale_and_recenter_mesh(Mesh & mesh);
static void init_camera_for_mesh(const Mesh & mesh, Camera & camera);
static void update_all(PoissonSolver & solver, Mesh & mesh, GPUMesh & mesh_gpu);
static void draw_scene(const Viewer & viewer, GLuint shader, const GPUMesh & gpu_mesh);
static void draw_gui(PoissonSolver & solver);
static void key_cb(int key, int action, int mods, void *args);
static void get_attr_bounds(const Mesh & m, float *attr_min, float *attr_max);


/*
 * Compiles the RHS expression entered by the user and evaluates it
 * at every mesh vertex to build the Poisson solver RHS vector f.
 */
bool new_rhs(PoissonSolver & solver) {
	
	srand((int)time(NULL));

	// Compile the mathematical expression entered by the user
	te_expr *test = te_compile(rhs_expression, rhs_vars, sizeof(rhs_vars) / sizeof(rhs_vars[0]), NULL);
	// Compilation failed, usually because of a syntax error
	if (!test)
		return false;

	// Replace the previous compiled expression
	te_free(te_rhs);
	te_rhs = test;

	// Evaluate f at every mesh vertex
	for (size_t i = 0; i < solver.N; ++i) {

		rhs_x = solver.m.positions[i].x;
		rhs_y = solver.m.positions[i].y;
		rhs_z = solver.m.positions[i].z;
		
		// Spherical coordinates used by the expression if needed
		rhs_p = atan2(rhs_y, rhs_x);
		rhs_t = atan2(sqrt(rhs_x * rhs_x + rhs_y * rhs_y), rhs_z);
		
		// Random value available through the "rand" variable
		rhs_r = (double)rand() / RAND_MAX;
		
		solver.f[i] = te_eval(te_rhs);
	}

	// Initialize the conjugate-gradient solver with the new RHS
	solver.init_cg();
	solver.iterate = 0;

	return true;
}

/*
 * Copies a vector of FEM values into mesh.attr.
 * mesh.attr is later uploaded to the GPU and used by the shader
 * to color the mesh.
 */

void transfer_to_mesh(const TArray<double> & V, Mesh & m) {
	// Mesh attributes are used to store the scalar value
	// that will be displayed by the GPU shader
	m.attr.resize(m.vertex_count());

	for (size_t i = 0; i < m.vertex_count(); ++i) {
		m.attr[i] = V[i];
	}
}


/*
 * Recenters the mesh at the origin and normalizes its size.
 * This makes the mesh easier to display independently of its original size.
 */

static void rescale_and_recenter_mesh(Mesh & mesh) {
	
	// Compute the bounding box of the mesh
	Aabb bbox = compute_mesh_bounds(mesh);

	// Find the center and the largest dimension of the mesh
	Vec3 model_center = (bbox.min + bbox.max) * 0.5f;
	Vec3 model_extent = (bbox.max - bbox.min);
	float model_size = max(model_extent);

	if (model_size == 0) {
		printf("Warning : Mesh is empty or reduced to a point.\n");
		model_size = 1;
	}

	// Center the mesh at the origin and normalize its size
	for (size_t i = 0; i < mesh.vertex_count(); ++i) {
		mesh.positions[i] -= model_center;
		mesh.positions[i] /= (model_size / 2);
	}
}


/*
 * Initializes the camera using the mesh bounding box.
 * The camera is placed in front of the mesh and points toward its center.
 */

static void init_camera_for_mesh(const Mesh & mesh, Camera & camera) {

	// Compute the mesh bounding box to position the camera automatically
	Aabb bbox = compute_mesh_bounds(mesh);
	Vec3 model_center = (bbox.min + bbox.max) * 0.5f;
	Vec3 model_extent = (bbox.max - bbox.min);
	float model_size = max(model_extent);
	
	if (model_size == 0) {
		printf("Warning : Mesh is empty or reduced to a point.\n");
		model_size = 1;
	}

	// Point the camera toward the center of the mesh
	camera.set_target(model_center);

	// Place the camera in front of the model
	Vec3 start_pos = (model_center + 2.f * Vec3(0, 0, model_size));
	camera.set_position(start_pos);

	// Set the visible depth range according to the mesh size
	camera.set_near(0.01 * model_size);
	camera.set_far(100 * model_size);
}

/*
 * Computes the minimum and maximum values stored in mesh.attr.
 * These values define the scalar range used by the color mapping.
 */
static void get_attr_bounds(const Mesh & m, float *attr_min, float *attr_max) {
	if (!m.vertex_count())
		return;

	// Find the minimum and maximum scalar values of the solution
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
 * Updates the Poisson solution and synchronizes the result with the GPU.
 * Depending on the state, this function performs CG iterations,
 * executes a reset, or does nothing.
 */
static void update_all(PoissonSolver & solver, Mesh & mesh, GPUMesh & gpu_mesh) {

	bool needs_upload = true;

	// Advance the solver either continuously or by one step
	if (started || one_step) {
		solver.do_iterate(iter_per_frame, 1e-6);

		// One-step mode is consumed after performing the iteration
		if (one_step) {
			one_step = false;
		}

		// Copy the current solution from the solver to the mesh
		transfer_to_mesh(solver.u, mesh);

		// Update the color range to match the current solution
		if (autoscale) {
			get_attr_bounds(mesh, &scale_min, &scale_max);
		}
	}
	else if (reset) {
		// Reset the solution and display the RHS again
		solver.clear_solution();
		transfer_to_mesh(solver.f, mesh);

		get_attr_bounds(mesh, &scale_min, &scale_max);
		reset = false;
	}
	else {
		// Nothing changed, so there is no need to update the GPU buffer
		needs_upload = false;
	}

	if (needs_upload) {
		// Send the updated scalar values to the GPU
		gpu_mesh.update_attr();
	}

	// Automatically stop the simulation when CG has converged
	if (solver.converged) {
		started = false;
	}
}

/*
 * Renders the FEM mesh using OpenGL.
 * The scalar solution stored in mesh.attr is converted to colors
 * by the FEM shader.
 */

static void draw_scene(const Viewer & viewer, GLuint shader, const GPUMesh & gpu_mesh) {
	// Clear the color and depth buffers at the beginning of each frame
	glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable depth testing so hidden triangles are not rendered over
	// triangles that are closer to the camera.
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	// Enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const Camera &camera = viewer.camera;

	// Activate the FEM visualization shader
	glUseProgram(shader);

	// Compute the transformation matrices used by the vertex shader
	Mat4 proj = camera.view_to_clip();
	Mat4 vm = camera.world_to_view();
	Vec3 camera_pos = camera.get_position();

	// Retrieve the locations of the shader uniforms
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

		// Avoid z-fighting between the surface and its edges
		float offset = reversed_z ? -1.f : 1.f;
		glPolygonOffset(offset, offset);

		// Draw the mesh as filled triangles with lighting
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glUniform1i(lighting_loc, GL_TRUE);

		gpu_mesh.draw();
	}

	if (draw_edges) {
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.f, 0.f);

		// Draw the mesh edges as lines without lighting
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform1i(lighting_loc, GL_FALSE);

		gpu_mesh.draw();
	}

	// Restore the default polygon mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_BLEND);
}


/*
 * Draws the ImGui control panel.
 * It allows the user to modify the RHS, control the CG solver,
 * and change the visualization parameters.
 */

static void draw_gui(PoissonSolver & solver) {
	ImGui::Begin("Controls");

	ImGui::Text("Solves -\\Delta u = f");
	ImGui::Text("--------------------");

	ImGui::Text("Enter math expression for f below:");
	ImGui::Text("(available variables : x, y, z, theta, phi, rand)");

	// Text field used to edit the RHS expression
	ImGui::InputText("", rhs_expression, IM_ARRAYSIZE(rhs_expression));

	if (ImGui::Button("Apply")) {
		// Compile and apply the new RHS expression
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
	ImGui::Text("Shows f at iter 0, then successive u_n iterates of cg.");
	ImGui::Text(" ");

	// Start the iterative solver
	if (ImGui::Button("Start")) {
		started = true;
	}

	ImGui::SameLine();

	// Stop the solver without resetting its current solution
	if (ImGui::Button("Stop")) {
		started = false;
	}

	ImGui::SameLine();

	// Perform one CG iteration when the solver is stopped
	if (ImGui::Button("One step")) {
		if (!started) {
			one_step = true;
		}
	}

	ImGui::SameLine();

	// Reset the solution to the initial RHS state
	if (ImGui::Button("Reset")) {
		reset = true;
	}

	ImGui::Text(" ");
	ImGui::Text("Iterate : %zu", solver.iterate);
	ImGui::Text("Relative error : %g", solver.rel_error);
	ImGui::Text("Scale min %.2f Scale max %.2f  (Span : %g)", scale_min, scale_max, scale_max - scale_min);

	ImGui::Text(" ");
	ImGui::Text("Controls :");

	ImGui::Checkbox("Autoscale", &autoscale);
	ImGui::Checkbox("Show edges", &draw_edges);

	ImGui::Text("Iterations per frame :");
	ImGui::DragInt(" ", &iter_per_frame, 1, 1, 20);

	ImGui::Text("Artificially deform mesh according to u :");
	ImGui::Text("(may help visualize oscillations of u)");
	ImGui::DragFloat("  ", &mesh_deform, 0.01f, 0.f, 1.f);

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
 * Handles keyboard shortcuts used by the viewer.
 * S toggles the surface and E toggles the mesh edges.
 */

static void key_cb(int key, int action, int mods, void *args) {
	(void)mods;
	(void)args;

	// Toggle surface rendering with the S key
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		draw_surface = !draw_surface;
		return;
	}

	// Toggle edge rendering with the E key
	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		draw_edges = !draw_edges;
		return;
	}
}

/*
#######################################################################################################
#######################################################################################################
* Initializes the Poisson solver, OpenGL viewer and GPU resources,
* then runs the main visualization loop.
#######################################################################################################
#######################################################################################################
*/


int main() {
	log_init(0);

	// Load the initial FEM mesh
	Mesh mesh;
	if (load_cube(mesh, 20)) {
		LOG_MSG("Error loading cube mesh.");
		exit(EXIT_FAILURE);
	}

	LOG_MSG("Loaded mesh.");

	// Center and normalize the mesh for visualization
	rescale_and_recenter_mesh(mesh);
	LOG_MSG("Mesh rescaled and recentered.");

	// Prepare the Poisson solver and evaluate the initial RHS
	PoissonSolver solver(mesh);

	if (!new_rhs(solver)) {
		LOG_MSG("Error loading rhs (expression flawed ?).");
		exit(EXIT_FAILURE);
	}

	// Initially display f on the mesh
	transfer_to_mesh(solver.f, mesh);
	get_attr_bounds(mesh, &scale_min, &scale_max);

	LOG_MSG("Prepared FEM data.");

	// Create the viewer and position the camera automatically
	Viewer viewer;
	init_camera_for_mesh(mesh, viewer.camera);
	viewer.init("Poisson solver");

	// Load the OpenGL functions after the context has been created
	if (!init_gl()) {
		LOG_MSG("Error initializing OpenGL functions.");
		exit(EXIT_FAILURE);
	}

	viewer.register_key_callback({ key_cb, NULL });
	LOG_MSG("Viewer initialized.");

	// Load the shaders used to display the FEM solution
	const char *vert_shader = "./shaders/fem.vert";
	const char *frag_shader = "./shaders/fem.frag";

	GLuint shader = create_shader(vert_shader, frag_shader);

	if (!shader) {
		exit(EXIT_FAILURE);
	}

	LOG_MSG("Shader initialized.");

	// Upload the mesh geometry and scalar attributes to the GPU
	GPUMesh gpu_mesh;
	gpu_mesh.m = &mesh;
	gpu_mesh.upload();

	// Main application loop.
	while (!viewer.should_close()) {
		// Process keyboard and mouse events
		viewer.poll_events();

		// Advance the FEM solver and update GPU data if necessary
		update_all(solver, mesh, gpu_mesh);

		// Start a new ImGui/OpenGL frame
		viewer.begin_frame();

		// Render the FEM solution and the control panel
		draw_scene(viewer, shader, gpu_mesh);
		draw_gui(solver);

		// Display the completed frame
		viewer.end_frame();
	}

	// Release viewer resources
	viewer.fini();

	log_fini();

	return (EXIT_SUCCESS);
}