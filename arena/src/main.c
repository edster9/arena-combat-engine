/*
 * Arena - Vehicular Combat Game
 * Combined editor and game - pause to plan, play to execute
 *
 * Milestone E3: Arena Walls + Obstacles
 * Milestone E4: Placeholder Cars for Scale Testing
 * Milestone E5: OBJ Loading + Car Rotation
 * Milestone E6: Entity System + Click Selection
 */

#include "platform/platform.h"
#include "math/vec3.h"
#include "math/mat4.h"
#include "render/camera.h"
#include "render/floor.h"
#include "render/mesh.h"
#include "render/obj_loader.h"
#include "game/entity.h"

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// PI constant for rotation
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define WINDOW_TITLE "Arena"

// Arena dimensions
#define ARENA_SIZE 60.0f
#define WALL_HEIGHT 4.0f
#define WALL_THICKNESS 1.0f

// Draw arena walls
static void draw_arena_walls(BoxRenderer* r) {
    Vec3 wall_color = vec3(0.5f, 0.45f, 0.4f);  // Concrete grey-brown
    float half = ARENA_SIZE / 2.0f;
    float y = WALL_HEIGHT / 2.0f;

    // North wall (positive Z)
    box_renderer_draw(r,
        vec3(0, y, half + WALL_THICKNESS/2),
        vec3(ARENA_SIZE + WALL_THICKNESS*2, WALL_HEIGHT, WALL_THICKNESS),
        wall_color);

    // South wall (negative Z)
    box_renderer_draw(r,
        vec3(0, y, -half - WALL_THICKNESS/2),
        vec3(ARENA_SIZE + WALL_THICKNESS*2, WALL_HEIGHT, WALL_THICKNESS),
        wall_color);

    // East wall (positive X)
    box_renderer_draw(r,
        vec3(half + WALL_THICKNESS/2, y, 0),
        vec3(WALL_THICKNESS, WALL_HEIGHT, ARENA_SIZE),
        wall_color);

    // West wall (negative X)
    box_renderer_draw(r,
        vec3(-half - WALL_THICKNESS/2, y, 0),
        vec3(WALL_THICKNESS, WALL_HEIGHT, ARENA_SIZE),
        wall_color);
}

// Car dimensions for scale testing (in game units)
// Using approx 1 unit = 1 meter scale
#define CAR_LENGTH 4.5f   // ~15 feet
#define CAR_WIDTH 2.0f    // ~6.5 feet
#define CAR_HEIGHT 1.4f   // ~4.5 feet (body only, not including roof)
#define CAR_ROOF_HEIGHT 0.5f

// Draw a placeholder box-car with body and cabin
static void draw_placeholder_car(BoxRenderer* r, Vec3 pos, float rotation_y, Vec3 body_color) {
    (void)rotation_y;  // TODO: implement rotation

    // Car body (lower part)
    Vec3 body_pos = vec3(pos.x, pos.y + CAR_HEIGHT/2, pos.z);
    box_renderer_draw(r, body_pos, vec3(CAR_LENGTH, CAR_HEIGHT, CAR_WIDTH), body_color);

    // Cabin/roof (upper part, smaller)
    Vec3 cabin_pos = vec3(pos.x - 0.3f, pos.y + CAR_HEIGHT + CAR_ROOF_HEIGHT/2, pos.z);
    Vec3 cabin_color = vec3(0.2f, 0.2f, 0.25f);  // Dark glass color
    box_renderer_draw(r, cabin_pos, vec3(CAR_LENGTH * 0.5f, CAR_ROOF_HEIGHT, CAR_WIDTH * 0.9f), cabin_color);

    // Wheels (4 corners) - small dark boxes for now
    Vec3 wheel_color = vec3(0.15f, 0.15f, 0.15f);
    float wheel_radius = 0.4f;
    float wheel_width = 0.3f;
    float wx = CAR_LENGTH/2 - 0.7f;  // Wheel X offset from center
    float wz = CAR_WIDTH/2 + wheel_width/2;  // Wheel Z offset

    // Front wheels
    box_renderer_draw(r, vec3(pos.x + wx, wheel_radius, pos.z + wz),
                      vec3(wheel_radius*2, wheel_radius*2, wheel_width), wheel_color);
    box_renderer_draw(r, vec3(pos.x + wx, wheel_radius, pos.z - wz),
                      vec3(wheel_radius*2, wheel_radius*2, wheel_width), wheel_color);
    // Rear wheels
    box_renderer_draw(r, vec3(pos.x - wx, wheel_radius, pos.z + wz),
                      vec3(wheel_radius*2, wheel_radius*2, wheel_width), wheel_color);
    box_renderer_draw(r, vec3(pos.x - wx, wheel_radius, pos.z - wz),
                      vec3(wheel_radius*2, wheel_radius*2, wheel_width), wheel_color);
}

// Draw obstacle blocks in the arena
static void draw_obstacles(BoxRenderer* r) {
    Vec3 barrier_color = vec3(0.7f, 0.3f, 0.2f);   // Rusty red
    Vec3 crate_color = vec3(0.6f, 0.5f, 0.3f);     // Wooden brown
    Vec3 pillar_color = vec3(0.4f, 0.4f, 0.45f);   // Dark concrete

    // Central barriers (cross pattern)
    box_renderer_draw(r, vec3(0, 1.0f, 0), vec3(8, 2, 2), barrier_color);
    box_renderer_draw(r, vec3(0, 1.0f, 0), vec3(2, 2, 8), barrier_color);

    // Corner pillars
    float corner_offset = 20.0f;
    box_renderer_draw(r, vec3(corner_offset, 2.0f, corner_offset), vec3(3, 4, 3), pillar_color);
    box_renderer_draw(r, vec3(-corner_offset, 2.0f, corner_offset), vec3(3, 4, 3), pillar_color);
    box_renderer_draw(r, vec3(corner_offset, 2.0f, -corner_offset), vec3(3, 4, 3), pillar_color);
    box_renderer_draw(r, vec3(-corner_offset, 2.0f, -corner_offset), vec3(3, 4, 3), pillar_color);

    // Scattered crates
    box_renderer_draw(r, vec3(10, 1.0f, 5), vec3(2, 2, 2), crate_color);
    box_renderer_draw(r, vec3(-12, 1.0f, -8), vec3(2, 2, 2), crate_color);
    box_renderer_draw(r, vec3(8, 1.0f, -15), vec3(2, 2, 2), crate_color);
    box_renderer_draw(r, vec3(-5, 1.0f, 12), vec3(2, 2, 2), crate_color);

    // Long barrier walls
    box_renderer_draw(r, vec3(15, 1.5f, 0), vec3(1.5f, 3, 12), barrier_color);
    box_renderer_draw(r, vec3(-15, 1.5f, 0), vec3(1.5f, 3, 12), barrier_color);
}

// Draw all vehicle entities
static void draw_entities(BoxRenderer* r, EntityManager* em, LoadedMesh* car_mesh) {
    for (int i = 0; i < em->count; i++) {
        Entity* e = &em->entities[i];
        if (!e->active || e->type != ENTITY_VEHICLE) continue;

        // Use highlight color if selected, otherwise normal team color
        Vec3 color = e->selected ? team_get_highlight_color(e->team)
                                 : team_get_color(e->team);

        if (car_mesh->valid) {
            box_renderer_draw_mesh(r, car_mesh->vao, car_mesh->vertex_count,
                                   e->position, e->scale, e->rotation_y, color);
        } else {
            // Fallback to placeholder
            draw_placeholder_car(r, e->position, e->rotation_y, color);
        }
    }
}

// Create initial test vehicles
static void create_test_vehicles(EntityManager* em, float car_scale) {
    Entity* e;

    // Red team
    e = entity_manager_create(em, ENTITY_VEHICLE, TEAM_RED);
    e->position = vec3(5, 0, 8);
    e->rotation_y = 0.0f;
    e->scale = car_scale;

    e = entity_manager_create(em, ENTITY_VEHICLE, TEAM_RED);
    e->position = vec3(-8, 0, 5);
    e->rotation_y = (float)(M_PI * 0.5);
    e->scale = car_scale;

    // Blue team
    e = entity_manager_create(em, ENTITY_VEHICLE, TEAM_BLUE);
    e->position = vec3(-5, 0, -10);
    e->rotation_y = (float)M_PI;
    e->scale = car_scale;

    e = entity_manager_create(em, ENTITY_VEHICLE, TEAM_BLUE);
    e->position = vec3(10, 0, -6);
    e->rotation_y = (float)(M_PI * 1.5);
    e->scale = car_scale;

    // Yellow team
    e = entity_manager_create(em, ENTITY_VEHICLE, TEAM_YELLOW);
    e->position = vec3(20, 0, 15);
    e->rotation_y = (float)(M_PI * 0.25);
    e->scale = car_scale;

    // Green team
    e = entity_manager_create(em, ENTITY_VEHICLE, TEAM_GREEN);
    e->position = vec3(-22, 0, -18);
    e->rotation_y = (float)(M_PI * 1.25);
    e->scale = car_scale;

    printf("Created %d vehicles\n", em->count);
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    printf("=== Arena ===\n");
    printf("Controls:\n");
    printf("  Left-click: Select vehicle\n");
    printf("  Right-click + drag: Look around\n");
    printf("  WASD: Move\n");
    printf("  E/Space: Move up\n");
    printf("  Q/Ctrl: Move down\n");
    printf("  Shift: Move faster\n");
    printf("  Scroll: Adjust move speed\n");
    printf("  F11: Toggle fullscreen\n");
    printf("  ESC: Quit\n\n");

    // Initialize platform
    Platform platform;
    if (!platform_init(&platform, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT)) {
        fprintf(stderr, "Failed to initialize platform\n");
        return 1;
    }

    // Initialize camera
    FlyCamera camera;
    camera_init(&camera);

    // Initialize floor (200 units total, 1 unit grid = Car Wars scale)
    Floor arena_floor;
    if (!floor_init(&arena_floor, 200.0f, 1.0f)) {
        fprintf(stderr, "Failed to initialize floor\n");
        platform_shutdown(&platform);
        return 1;
    }

    // Initialize box renderer for walls and obstacles
    BoxRenderer box_renderer;
    if (!box_renderer_init(&box_renderer)) {
        fprintf(stderr, "Failed to initialize box renderer\n");
        floor_destroy(&arena_floor);
        platform_shutdown(&platform);
        return 1;
    }

    // Load car model
    LoadedMesh car_mesh;
    float car_scale = 1.0f;
    if (obj_load(&car_mesh, "assets/models/vehicles/kenney-car-kit/Models/OBJ format/sedan-sports.obj")) {
        // Calculate scale to make car approximately CAR_LENGTH long
        Vec3 model_size = obj_get_size(&car_mesh);
        float model_length = fmaxf(model_size.x, model_size.z);  // Use longest horizontal axis
        if (model_length > 0.001f) {
            car_scale = CAR_LENGTH / model_length;
        }
        printf("Loaded car model: %.1f x %.1f x %.1f, scale: %.2f\n",
               model_size.x, model_size.y, model_size.z, car_scale);
    } else {
        printf("Warning: Could not load car model, using placeholders\n");
    }

    // Initialize entity manager and create test vehicles
    EntityManager entities;
    entity_manager_init(&entities);
    create_test_vehicles(&entities, car_scale);

    // Light direction (sun from upper-right-front)
    Vec3 light_dir = vec3_normalize(vec3(0.5f, -1.0f, 0.3f));

    // Input state
    InputState input;
    memset(&input, 0, sizeof(InputState));

    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);

    // Timing
    double last_time = platform_get_time();
    int frame_count = 0;
    double fps_timer = 0;

    // Main loop
    while (!platform.should_quit) {
        // Timing
        double current_time = platform_get_time();
        float dt = (float)(current_time - last_time);
        last_time = current_time;

        // FPS counter
        frame_count++;
        fps_timer += dt;
        if (fps_timer >= 1.0) {
            char title[128];
            snprintf(title, sizeof(title), "%s | FPS: %d | Pos: (%.1f, %.1f, %.1f)",
                     WINDOW_TITLE, frame_count,
                     camera.position.x, camera.position.y, camera.position.z);
            platform_set_title(&platform, title);
            frame_count = 0;
            fps_timer = 0;
        }

        // Input
        platform_poll_events(&platform, &input);

        // Toggle mouse capture with right click
        if (input.mouse_pressed[MOUSE_RIGHT]) {
            platform_capture_mouse(&platform, &input, true);
        }
        if (input.mouse_released[MOUSE_RIGHT]) {
            platform_capture_mouse(&platform, &input, false);
        }

        // Fullscreen toggle
        if (input.keys_pressed[KEY_F11]) {
            platform_toggle_fullscreen(&platform);
        }

        // Quit on ESC
        if (input.keys_pressed[KEY_ESCAPE]) {
            platform.should_quit = true;
        }

        // Left click to select vehicles (only when not looking around)
        if (input.mouse_pressed[MOUSE_LEFT] && !input.mouse_captured) {
            Vec3 ray_origin, ray_dir;
            camera_screen_to_ray(&camera, input.mouse_x, input.mouse_y,
                                 platform.width, platform.height,
                                 &ray_origin, &ray_dir);

            int hit_id = entity_manager_pick(&entities, ray_origin, ray_dir);
            if (hit_id >= 0) {
                entity_manager_select(&entities, hit_id);
                Entity* selected = entity_manager_get_selected(&entities);
                if (selected) {
                    printf("Selected: %s team vehicle at (%.1f, %.1f)\n",
                           selected->team == TEAM_RED ? "Red" :
                           selected->team == TEAM_BLUE ? "Blue" :
                           selected->team == TEAM_YELLOW ? "Yellow" : "Green",
                           selected->position.x, selected->position.z);
                }
            } else {
                entity_manager_deselect_all(&entities);
            }
        }

        // Update camera
        camera_update(&camera, &input, dt);

        // Render
        glViewport(0, 0, platform.width, platform.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up matrices
        float aspect = (float)platform.width / (float)platform.height;
        Mat4 projection = camera_projection_matrix(&camera, aspect);
        Mat4 view = camera_view_matrix(&camera);

        // Draw floor with procedural grid (modern OpenGL shader)
        floor_render(&arena_floor, &view, &projection, camera.position);

        // Draw walls, obstacles, and cars with lighting
        box_renderer_begin(&box_renderer, &view, &projection, light_dir);
        draw_arena_walls(&box_renderer);
        draw_obstacles(&box_renderer);
        draw_entities(&box_renderer, &entities, &car_mesh);
        box_renderer_end(&box_renderer);

        // Swap buffers
        platform_swap_buffers(&platform);
    }

    // Cleanup
    obj_destroy(&car_mesh);
    box_renderer_destroy(&box_renderer);
    floor_destroy(&arena_floor);
    platform_shutdown(&platform);
    printf("Goodbye!\n");

    return 0;
}
