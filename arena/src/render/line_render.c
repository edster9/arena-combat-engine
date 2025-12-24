#include "line_render.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Vertex shader for lines
static const char* line_vertex_shader =
    "#version 330 core\n"
    "layout(location = 0) in vec3 a_pos;\n"
    "\n"
    "uniform mat4 u_view;\n"
    "uniform mat4 u_projection;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = u_projection * u_view * vec4(a_pos, 1.0);\n"
    "}\n";

// Fragment shader for lines
static const char* line_fragment_shader =
    "#version 330 core\n"
    "uniform vec4 u_color;\n"
    "out vec4 frag_color;\n"
    "\n"
    "void main() {\n"
    "    frag_color = u_color;\n"
    "}\n";

static GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Line Shader compile error: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool line_renderer_init(LineRenderer* lr) {
    // Compile shaders
    GLuint vert = compile_shader(GL_VERTEX_SHADER, line_vertex_shader);
    if (!vert) return false;

    GLuint frag = compile_shader(GL_FRAGMENT_SHADER, line_fragment_shader);
    if (!frag) {
        glDeleteShader(vert);
        return false;
    }

    // Link program
    lr->shader_program = glCreateProgram();
    glAttachShader(lr->shader_program, vert);
    glAttachShader(lr->shader_program, frag);
    glLinkProgram(lr->shader_program);

    GLint success;
    glGetProgramiv(lr->shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(lr->shader_program, sizeof(log), NULL, log);
        fprintf(stderr, "Line Shader link error: %s\n", log);
        glDeleteShader(vert);
        glDeleteShader(frag);
        glDeleteProgram(lr->shader_program);
        return false;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    // Get uniform locations
    lr->u_view = glGetUniformLocation(lr->shader_program, "u_view");
    lr->u_projection = glGetUniformLocation(lr->shader_program, "u_projection");
    lr->u_color = glGetUniformLocation(lr->shader_program, "u_color");

    // Create VAO/VBO
    glGenVertexArrays(1, &lr->vao);
    glGenBuffers(1, &lr->vbo);

    glBindVertexArray(lr->vao);
    glBindBuffer(GL_ARRAY_BUFFER, lr->vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_LINE_POINTS * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    printf("Line Renderer initialized\n");
    return true;
}

void line_renderer_destroy(LineRenderer* lr) {
    if (lr->vao) glDeleteVertexArrays(1, &lr->vao);
    if (lr->vbo) glDeleteBuffers(1, &lr->vbo);
    if (lr->shader_program) glDeleteProgram(lr->shader_program);
}

void line_renderer_begin(LineRenderer* lr, Mat4* view, Mat4* projection) {
    glUseProgram(lr->shader_program);
    glUniformMatrix4fv(lr->u_view, 1, GL_FALSE, view->m);
    glUniformMatrix4fv(lr->u_projection, 1, GL_FALSE, projection->m);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(3.0f);

    glBindVertexArray(lr->vao);
}

void line_renderer_draw_line(LineRenderer* lr, Vec3 start, Vec3 end, Vec3 color, float alpha) {
    float vertices[] = {
        start.x, start.y, start.z,
        end.x, end.y, end.z
    };

    glBindBuffer(GL_ARRAY_BUFFER, lr->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glUniform4f(lr->u_color, color.x, color.y, color.z, alpha);
    glDrawArrays(GL_LINES, 0, 2);
}

void line_renderer_draw_path(LineRenderer* lr, Vec3* points, int count, Vec3 color, float alpha) {
    if (count < 2 || count > MAX_LINE_POINTS) return;

    float vertices[MAX_LINE_POINTS * 3];
    for (int i = 0; i < count; i++) {
        vertices[i * 3 + 0] = points[i].x;
        vertices[i * 3 + 1] = points[i].y;
        vertices[i * 3 + 2] = points[i].z;
    }

    glBindBuffer(GL_ARRAY_BUFFER, lr->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * 3 * sizeof(float), vertices);

    glUniform4f(lr->u_color, color.x, color.y, color.z, alpha);
    glDrawArrays(GL_LINE_STRIP, 0, count);
}

void line_renderer_draw_circle(LineRenderer* lr, Vec3 center, float radius, Vec3 color, float alpha) {
    const int segments = 24;
    float vertices[segments * 3];

    for (int i = 0; i < segments; i++) {
        float angle = (float)i / segments * 2.0f * M_PI;
        vertices[i * 3 + 0] = center.x + cosf(angle) * radius;
        vertices[i * 3 + 1] = center.y;
        vertices[i * 3 + 2] = center.z + sinf(angle) * radius;
    }

    glBindBuffer(GL_ARRAY_BUFFER, lr->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, segments * 3 * sizeof(float), vertices);

    glUniform4f(lr->u_color, color.x, color.y, color.z, alpha);
    glDrawArrays(GL_LINE_LOOP, 0, segments);
}

void line_renderer_end(LineRenderer* lr) {
    (void)lr;
    glBindVertexArray(0);
    glUseProgram(0);
    glLineWidth(1.0f);
}
