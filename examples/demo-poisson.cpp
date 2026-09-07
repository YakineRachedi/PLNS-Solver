#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstring>

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "tiny_expr/tinyexpr.h"

#include "math_utils.h"
#include "cube.h"
#include "mesh.h"
#include "poisson.h"


/******************************************************************************
 * Demo parameters
 *****************************************************************************/

static bool running = true;
static int iterations_per_frame = 5;
static double tolerance = 1e-6;

/*
 * Isometric view parameters.
 *
 * yaw   : rotation around the vertical (Y) axis, in radians.
 * pitch : tilt angle used to obtain the isometric look, in radians.
 *
 * The user can adjust yaw interactively; pitch is kept fixed at the
 * classic isometric angle but is exposed as a variable in case a future
 * version wants to make it adjustable too.
 */
static float view_yaw   = 0.78539816f; /* 45 degrees   */
static float view_pitch = 0.61547971f; /* ~35.26 degrees (isometric tilt) */


/******************************************************************************
 * Compute the minimum and maximum values of a scalar field.
 *
 * These values are used to normalize the solution before converting it into
 * colors.
 *****************************************************************************/

static void get_bounds(const double *V, size_t N, double & min_value, double & max_value) {
    if (N == 0) {
        min_value = 0.0;
        max_value = 0.0;
        return;
    }

    min_value = V[0];
    max_value = V[0];

    for (size_t i = 1; i < N; ++i) {

        if (V[i] < min_value)
            min_value = V[i];

        if (V[i] > max_value)
            max_value = V[i];
    }
}


/******************************************************************************
 * Convert a scalar value into a color.
 *
 * The color scale is:
 *
 *     low value  -> blue
 *     middle     -> green
 *     high value -> red
 *
 * The input value x is assumed to be normalized between 0 and 1.
 *****************************************************************************/

static ImU32 scalar_to_color(double x) {
    if (x < 0.0) x = 0.0;
    if (x > 1.0) x = 1.0;

    int r = 0;
    int g = 0;
    int b = 0;

    if (x < 0.5) {

        /*
         * Blue -> Green
         */

        double t = 2.0 * x;

        r = 0;
        g = static_cast<int>(255.0 * t);
        b = static_cast<int>(255.0 * (1.0 - t));

    } else {

        /*
         * Green -> Red
         */

        double t = 2.0 * (x - 0.5);

        r = static_cast<int>(255.0 * t);
        g = static_cast<int>(255.0 * (1.0 - t));
        b = 0;
    }

    return IM_COL32(r, g, b, 255);
}


/******************************************************************************
 * A vertex projected onto the screen.
 *
 * screen : 2D window-space coordinates, ready to be drawn by ImGui.
 * depth  : view-space depth after rotation, used later for back-to-front
 *          triangle sorting (the painter's algorithm). Larger values are
 *          farther from the viewer.
 *****************************************************************************/
struct ProjectedVertex {
    ImVec2 screen;
    float  depth;
};


/******************************************************************************
 * Project a 3D mesh vertex onto the 2D screen using an isometric view.
 *
 * Unlike a flat XY projection, this rotates the point around the vertical
 * (Y) axis by `yaw`, then tilts it by `pitch` (the classic isometric angle,
 * arctan(1/sqrt(2)) ~= 35.264 degrees). This way all three axes (X, Y, Z)
 * contribute visibly to the 2D projection, giving the mesh actual depth
 * instead of collapsing every face onto a single flat square.
 *
 * The resulting depth value (view-space Z after rotation) is also
 * returned, so that triangles can later be sorted back-to-front before
 * being drawn.
 *
 * @param p      Mesh vertex position, expected to lie roughly in [-1,1]^3.
 * @param origin Top-left corner of the drawing area, in screen coordinates.
 * @param size   Size of the drawing area, in screen pixels.
 * @param scale  Additional scale factor controlling the on-screen size of
 *               the mesh.
 * @param yaw    Rotation around the Y axis, in radians.
 * @param pitch  Tilt angle applied after the yaw rotation, in radians.
 *****************************************************************************/

static ProjectedVertex project_vertex(const Vec3 & p, ImVec2 origin, ImVec2 size,
                                       float scale, float yaw, float pitch) {

    /* Rotate around the Y (vertical) axis by `yaw`. */
    float cy = std::cos(yaw);
    float sy = std::sin(yaw);

    float x1 = p.x * cy + p.z * sy;
    float z1 = -p.x * sy + p.z * cy;
    float y1 = p.y;

    /* Tilt around the X axis by `pitch` to obtain the isometric look. */
    float cp = std::cos(pitch);
    float sp = std::sin(pitch);

    float y2 = y1 * cp - z1 * sp;
    float z2 = y1 * sp + z1 * cp;

    /* Map to screen coordinates, centered in the drawing area.
     * Screen Y grows downward, so the vertical axis is flipped. */
    float screen_x = origin.x + size.x * 0.5f + x1 * scale;
    float screen_y = origin.y + size.y * 0.5f - y2 * scale;

    ProjectedVertex out;
    out.screen = ImVec2(screen_x, screen_y);
    out.depth  = z2;

    return out;
}


/******************************************************************************
 * Draw the mesh and a scalar field using an isometric 3D projection.
 *
 * Each triangle receives a color corresponding to the average value of the
 * scalar field at its three vertices.
 *
 * Because ImGui's drawing API has no depth buffer, triangles are sorted
 * back-to-front (the classic painter's algorithm) before being drawn, so
 * that nearer faces correctly occlude farther ones.
 *
 * This is deliberately simple:
 *
 *     - no shaders;
 *     - no GPU mesh;
 *     - no OpenGL vertex buffers;
 *     - no texture.
 *
 * The visualization is performed using ImGui's drawing API.
 *****************************************************************************/

static void draw_scalar_field(const Mesh & mesh, const double *V, ImVec2 origin, ImVec2 size,
                               float scale, float yaw, float pitch) {

    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    size_t vtx_count = mesh.vertex_count();
    size_t tri_count = mesh.triangle_count();

    /* Compute the scalar range. */

    double min_value;
    double max_value;

    get_bounds(V, vtx_count, min_value, max_value);
    double range = max_value - min_value;

    if (std::abs(range) < 1e-15)
        range = 1.0;

    /*
     * Project every mesh vertex once, storing both its screen position
     * and its view-space depth.
     */
    std::vector<ProjectedVertex> projected(vtx_count);

    for (size_t i = 0; i < vtx_count; ++i) {
        projected[i] = project_vertex(mesh.positions[i], origin, size, scale, yaw, pitch);
    }

    /*
     * Compute the average depth of every triangle, then build an index
     * array sorted from farthest to nearest (painter's algorithm).
     */
    std::vector<float>  tri_depth(tri_count);
    std::vector<size_t> tri_order(tri_count);

    for (size_t t = 0; t < tri_count; ++t) {
        uint32_t a = mesh.indices[3 * t + 0];
        uint32_t b = mesh.indices[3 * t + 1];
        uint32_t c = mesh.indices[3 * t + 2];

        float depth = (projected[a].depth + projected[b].depth + projected[c].depth) / 3.0f;

        tri_depth[t] = depth;
        tri_order[t] = t;
    }

    /* Farthest triangles (largest depth) are drawn first. */
    std::sort(tri_order.begin(), tri_order.end(),
              [&](size_t i, size_t j) { return tri_depth[i] > tri_depth[j]; });

    /* Draw all triangles, back to front. */

    for (size_t k = 0; k < tri_count; ++k) {
        size_t t = tri_order[k];

        uint32_t a = mesh.indices[3 * t + 0];
        uint32_t b = mesh.indices[3 * t + 1];
        uint32_t c = mesh.indices[3 * t + 2];

        ImVec2 A = projected[a].screen;
        ImVec2 B = projected[b].screen;
        ImVec2 C = projected[c].screen;

        /* Compute the average solution value on the triangle */

        double value = (V[a] + V[b] + V[c]) / 3.0;

        /* Normalize the value */

        double normalized = (value - min_value) / range;

        /* Convert the value into a color. */

        ImU32 color = scalar_to_color(normalized);

        /* Draw the filled triangle */

        draw_list->AddTriangleFilled(A, B, C, color);

        /* Draw the triangle edges */

        draw_list->AddTriangle(A, B, C, IM_COL32(255, 255, 255, 100), 1.0f);
    }
}

/******************************************************************************
 * Initialize the right-hand side.
 *
 * The Poisson problem is:
 *
 *     -Delta(u) = f
 *
 * The mesh is the surface of a cube. Therefore we cannot use a product such
 * as sin(pi*x) * sin(pi*y) * sin(pi*z), because at every point of the cube
 * at least one coordinate is equal to +/-1 and the product would be zero.
 *
 * We therefore use a sum:
 *
 *     f(x,y,z) = sin(pi*x) + sin(pi*y) + sin(pi*z)
 *
 * This produces a non-zero scalar field over the cube surface.
 *****************************************************************************/

static void initialize_rhs(PoissonSolver &solver) {
    for (size_t i = 0; i < solver.N; ++i) {
        const Vec3 &p = solver.m.positions[i];

        solver.f[i] =
            std::sin(PI * p.x) +
            std::sin(PI * p.y) +
            std::sin(PI * p.z);
    }

    /*
     * The right-hand side has changed, so the Conjugate Gradient solver
     * must be initialized again from this new f.
     */
    solver.init_cg();

    solver.iterate = 0;
}

int main() {

    if (!glfwInit()) {

        printf("Error: GLFW initialization failed.\n");

        return EXIT_FAILURE;
    }

    // Create a window.

    GLFWwindow *window = glfwCreateWindow(1200, 800, "PLNS Solver - Poisson Demo", NULL, NULL);

    if (!window) {

        printf("Error: window creation failed.\n");
        glfwTerminate();

        return EXIT_FAILURE;
    }


    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO();

    (void)io;

    // Use the dark ImGui theme

    ImGui::StyleColorsDark();

    // Initialize GLFW and OpenGL backends

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");


    /**************************************************************************
     * Build the mesh.
     *
     * For now we use a cube surface.
     *
     * The cube is subdivided and duplicate vertices are removed by:
     *
     *     load_cube()
     *
     *************************************************************************/

    Mesh mesh;

    size_t subdiv = 20;

    if (load_cube(mesh, subdiv)) {

        printf("Error: unable to create mesh.\n");

        return EXIT_FAILURE;
    }

    printf("Mesh created:\n");

    printf("  vertices  : %zu\n", mesh.vertex_count());
    printf("  triangles : %zu\n", mesh.triangle_count());

    PoissonSolver solver(mesh);
    initialize_rhs(solver);

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        // Start a new ImGui frame

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Perform Poisson iterations
        if (running && !solver.converged) {
            solver.do_iterate(iterations_per_frame, tolerance);
        }

        // Stop automatically when convergence is reached
        if (solver.converged) running = false;

        ImGui::Begin("Poisson Solver");
        ImGui::Text("Finite Element Poisson Solver");
        ImGui::Separator();
        ImGui::Text("Equation:");
        ImGui::Text("-Delta u = f");
        ImGui::Separator();
        ImGui::Text("Mesh");
        ImGui::Text("Vertices: %zu", mesh.vertex_count());
        ImGui::Text("Triangles: %zu", mesh.triangle_count());
        ImGui::Separator();
        ImGui::Text("Solver");
        ImGui::Text("Iterations: %zu", solver.iterate);
        ImGui::Text("Relative residual: %.3e", solver.rel_error);
        ImGui::Text("Tolerance: %.3e", tolerance);
        ImGui::Separator();
        if (ImGui::Button("Start")) {running = true;}
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {running = false;}

        /* One iteration */

        if (ImGui::Button("One step")) {
            if (!solver.converged) {
                solver.do_iterate(1, tolerance);
            }
        }

        if (ImGui::Button("Reset")) {
            running = false;
            solver.clear_solution();
        }


        ImGui::Separator();
        ImGui::SliderInt("Iterations / frame", &iterations_per_frame, 1, 100);
        ImGui::Separator();
        ImGui::SliderAngle("View rotation", &view_yaw, -180.0f, 180.0f);
        ImGui::Separator();

        if (solver.converged) {
            ImGui::Text("Status: CONVERGED");
        }
        else if (running) {
            ImGui::Text("Status: RUNNING");
        }
        else {ImGui::Text("Status: STOPPED");}

        ImGui::End();

        // Visualization window.

        ImGui::SetNextWindowSize(
            ImVec2(700.0f, 700.0f),
            ImGuiCond_FirstUseEver
        );

        ImGui::Begin("Solution");

        /*
        * Get the space available inside the window.
        */
        ImVec2 available = ImGui::GetContentRegionAvail();

        /*
        * Keep a square visualization.
        */
        float area_size = std::min(available.x, available.y);

        /*
        * Avoid passing a zero-sized rectangle to ImGui.
        */
        if (area_size <= 0.0f) {
            ImGui::End();
        }
        else {

            ImVec2 origin = ImGui::GetCursorScreenPos();
            ImVec2 draw_size(area_size, area_size);

            /*
            * Scale the mesh.
            */
            float scale = area_size * 0.35f;

            /*
            * Create the interactive area.
            */
            ImGui::InvisibleButton(
                "view_drag_area",
                draw_size
            );

            /*
            * Draw the visualization.
            */
            ImDrawList *draw_list =
                ImGui::GetWindowDrawList();

            draw_list->AddRectFilled(
                origin,
                ImVec2(
                    origin.x + draw_size.x,
                    origin.y + draw_size.y
                ),
                IM_COL32(30, 30, 30, 255)
            );

            draw_scalar_field(
                mesh,
                solver.u.data,
                origin,
                draw_size,
                scale,
                view_yaw,
                view_pitch
            );

            /*
            * Rotate the view with the mouse.
            */
            if (ImGui::IsItemActive() &&
                ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

                ImVec2 delta =
                    ImGui::GetIO().MouseDelta;

                view_yaw += delta.x * 0.01f;
            }

        ImGui::End();
        }
        ImGui::Render();


        int display_width;
        int display_height;


        glfwGetFramebufferSize(window, &display_width, &display_height);
        glViewport(0, 0, display_width, display_height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}