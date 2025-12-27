/*
 * 3D Hardware Acceleration Test for WSL2
 * Tests OpenGL rendering with a spinning cube and FPS counter
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Cube vertices (position only)
static const float cubeVertices[] = {
    // Front face
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    // Back face
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
};

// Cube face indices
static const unsigned int cubeIndices[] = {
    // Front
    0, 1, 2,  2, 3, 0,
    // Right
    1, 5, 6,  6, 2, 1,
    // Back
    5, 4, 7,  7, 6, 5,
    // Left
    4, 0, 3,  3, 7, 4,
    // Top
    3, 2, 6,  6, 7, 3,
    // Bottom
    4, 5, 1,  1, 0, 4,
};

// Face colors (RGBA)
static const float faceColors[6][4] = {
    {1.0f, 0.0f, 0.0f, 1.0f},  // Front - Red
    {0.0f, 1.0f, 0.0f, 1.0f},  // Right - Green
    {0.0f, 0.0f, 1.0f, 1.0f},  // Back - Blue
    {1.0f, 1.0f, 0.0f, 1.0f},  // Left - Yellow
    {1.0f, 0.0f, 1.0f, 1.0f},  // Top - Magenta
    {0.0f, 1.0f, 1.0f, 1.0f},  // Bottom - Cyan
};

void printOpenGLInfo(void) {
    printf("\n=== OpenGL Hardware Info ===\n");
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    printf("GLSL:     %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Check if we're using hardware acceleration
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    if (renderer) {
        if (strstr(renderer, "llvmpipe") || strstr(renderer, "softpipe") ||
            strstr(renderer, "Software") || strstr(renderer, "CPU")) {
            printf("\n⚠️  WARNING: Software rendering detected!\n");
            printf("   Hardware acceleration may not be working.\n");
        } else {
            printf("\n✅ Hardware acceleration appears to be working!\n");
        }
    }
    printf("============================\n\n");
}

void setupProjection(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)width / (float)height;
    float fov = 45.0f;
    float near = 0.1f;
    float far = 100.0f;

    float top = near * tanf(fov * 3.14159265f / 360.0f);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    glFrustum(left, right, bottom, top, near, far);
    glMatrixMode(GL_MODELVIEW);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void)window;  // unused
    setupProjection(width, height);
}

void drawCube(float angle) {
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.0f);
    glRotatef(angle, 1.0f, 1.0f, 0.0f);

    glBegin(GL_TRIANGLES);
    for (int face = 0; face < 6; face++) {
        glColor4fv(faceColors[face]);
        for (int i = 0; i < 6; i++) {
            int idx = cubeIndices[face * 6 + i];
            glVertex3f(cubeVertices[idx * 3],
                       cubeVertices[idx * 3 + 1],
                       cubeVertices[idx * 3 + 2]);
        }
    }
    glEnd();
}

int main(void) {
    printf("Starting 3D Hardware Acceleration Test...\n");

    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Create window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
        "3D Hardware Test - WSL2", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
        return -1;
    }

    // Print OpenGL info
    printOpenGLInfo();

    // Setup OpenGL state
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    setupProjection(WINDOW_WIDTH, WINDOW_HEIGHT);

    // FPS tracking
    double lastTime = glfwGetTime();
    int frameCount = 0;
    float angle = 0.0f;

    printf("Rendering spinning cube... Press ESC or close window to exit.\n");
    printf("Watch the FPS - should be 60+ for hardware acceleration.\n\n");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate FPS
        double currentTime = glfwGetTime();
        frameCount++;

        if (currentTime - lastTime >= 1.0) {
            char title[128];
            snprintf(title, sizeof(title),
                "3D Hardware Test - WSL2 | FPS: %d | %s",
                frameCount, glGetString(GL_RENDERER));
            glfwSetWindowTitle(window, title);

            printf("FPS: %d\n", frameCount);
            frameCount = 0;
            lastTime = currentTime;
        }

        // Clear and draw
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        angle += 1.0f;
        if (angle >= 360.0f) angle -= 360.0f;

        drawCube(angle);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // ESC to exit
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    printf("\nTest complete!\n");
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
