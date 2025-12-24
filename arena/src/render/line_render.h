#ifndef LINE_RENDER_H
#define LINE_RENDER_H

#include <GL/glew.h>
#include <stdbool.h>
#include "../math/vec3.h"
#include "../math/mat4.h"

/*
 * Line Renderer
 * =============
 * Draws lines and paths in 3D space.
 * Used for movement path preview.
 */

#define MAX_LINE_POINTS 64

typedef struct {
    GLuint shader_program;
    GLuint vao;
    GLuint vbo;

    // Uniform locations
    GLint u_view;
    GLint u_projection;
    GLint u_color;
} LineRenderer;

// Initialize the line renderer
bool line_renderer_init(LineRenderer* lr);

// Cleanup
void line_renderer_destroy(LineRenderer* lr);

// Begin rendering lines (call once per frame before drawing)
void line_renderer_begin(LineRenderer* lr, Mat4* view, Mat4* projection);

// Draw a line between two points
void line_renderer_draw_line(LineRenderer* lr, Vec3 start, Vec3 end, Vec3 color, float alpha);

// Draw a path (series of connected points)
void line_renderer_draw_path(LineRenderer* lr, Vec3* points, int count, Vec3 color, float alpha);

// Draw a circle on the ground (for waypoints/markers)
void line_renderer_draw_circle(LineRenderer* lr, Vec3 center, float radius, Vec3 color, float alpha);

// End rendering
void line_renderer_end(LineRenderer* lr);

#endif // LINE_RENDER_H
