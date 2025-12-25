/*
 * Arena - Vehicular Combat Game
 * Combined editor and game - pause to plan, play to execute
 *
 * Milestone E7: Turn Planning UI + Execute (Teleport)
 * Milestone E8: Freestyle Physics Mode
 * Milestone E9: ODE Physics with Suspension
 */

#include "platform/platform.h"
#include "math/vec3.h"
#include "math/mat4.h"
#include "render/camera.h"
#include "render/floor.h"
#include "render/mesh.h"
#include "render/obj_loader.h"
#include "game/entity.h"
#include "render/line_render.h"
#include "ui/ui_render.h"
#include "ui/ui_text.h"
#include "physics/ode_physics.h"

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

// Game modes
typedef enum {
    MODE_TURN_BASED = 0,
    MODE_FREESTYLE = 1
} GameMode;

// Planning UI state (for turn-based mode)
typedef enum {
    SPEED_BRAKE = 0,
    SPEED_HOLD = 1,
    SPEED_ACCEL = 2
} SpeedChoice;

typedef struct {
    SpeedChoice speed_choice;
    int selected_phase;      // 0-4, which phase box is selected
    int current_speed;       // Current speed in mph
} PlanningState;

// Physics state for freestyle mode
typedef struct {
    float velocity;          // Current speed in game units/sec
    float angular_velocity;  // Turning rate in radians/sec
    float steering;          // Current steering input (-1 to 1)
} CarPhysics;

// Physics constants
#define CAR_ACCEL 15.0f          // Acceleration in units/sec^2
#define CAR_BRAKE 25.0f          // Braking deceleration
#define CAR_FRICTION 3.0f        // Natural deceleration (drag)
#define CAR_MAX_SPEED 40.0f      // Max speed in units/sec (~90 mph in game scale)
#define CAR_TURN_RATE 2.5f       // Max turn rate in radians/sec
#define CAR_MIN_TURN_SPEED 2.0f  // Minimum speed to turn

// Calculate next speed based on choice
static int calculate_next_speed(int current_speed, SpeedChoice choice) {
    switch (choice) {
        case SPEED_BRAKE: return current_speed > 0 ? current_speed - 5 : 0;
        case SPEED_ACCEL: return current_speed + 5;
        case SPEED_HOLD:
        default: return current_speed;
    }
}

// Calculate movement distance in game units for a given speed
// Car Wars: distance per turn = speed / 10 inches
// Our scale: 1 game unit = 1 inch
static float calculate_move_distance(int speed_mph) {
    return (float)speed_mph / 10.0f;
}

// Calculate end position for a straight-line move
static Vec3 calculate_end_position(Vec3 start, float rotation_y, float distance) {
    // Forward direction based on rotation
    float dx = sinf(rotation_y) * distance;
    float dz = cosf(rotation_y) * distance;
    return vec3(start.x + dx, start.y, start.z + dz);
}

// Check if point is inside rect
static bool point_in_rect(float px, float py, UIRect rect) {
    return px >= rect.x && px <= rect.x + rect.width &&
           py >= rect.y && py <= rect.y + rect.height;
}

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

// Draw obstacle blocks in the arena (simplified for showdown)
static void draw_obstacles(BoxRenderer* r) {
    Vec3 pillar_color = vec3(0.4f, 0.4f, 0.45f);   // Dark concrete
    Vec3 barrier_color = vec3(0.6f, 0.35f, 0.25f); // Rusty barrier

    // Central pillar - forces cars to maneuver around
    box_renderer_draw(r, vec3(0, 2.0f, 0), vec3(5, 4, 5), barrier_color);

    // Corner pillars
    float corner_offset = 22.0f;
    box_renderer_draw(r, vec3(corner_offset, 2.0f, corner_offset), vec3(3, 4, 3), pillar_color);
    box_renderer_draw(r, vec3(-corner_offset, 2.0f, corner_offset), vec3(3, 4, 3), pillar_color);
    box_renderer_draw(r, vec3(corner_offset, 2.0f, -corner_offset), vec3(3, 4, 3), pillar_color);
    box_renderer_draw(r, vec3(-corner_offset, 2.0f, -corner_offset), vec3(3, 4, 3), pillar_color);
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

// Create showdown vehicles - 2 cars facing each other
static void create_test_vehicles(EntityManager* em, float car_scale) {
    Entity* e;

    // Red car - south side near wall, facing north (toward blue)
    e = entity_manager_create(em, ENTITY_VEHICLE, TEAM_RED);
    e->position = vec3(0, 0, -26);
    e->rotation_y = 0.0f;  // Facing +Z (north)
    e->scale = car_scale;

    // Blue car - north side near wall, facing south (toward red)
    e = entity_manager_create(em, ENTITY_VEHICLE, TEAM_BLUE);
    e->position = vec3(0, 0, 26);
    e->rotation_y = (float)M_PI;  // Facing -Z (south)
    e->scale = car_scale;

    printf("Created %d vehicles (showdown mode)\n", em->count);
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    printf("=== Arena ===\n");
    printf("Press F1 for controls help\n\n");

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

    // Initialize ODE physics
    PhysicsWorld physics;
    if (!physics_init(&physics)) {
        fprintf(stderr, "Failed to initialize physics\n");
        // Continue anyway, physics just won't work
    }

    // Set up ground plane
    physics_set_ground(&physics, 0.0f);

    // Add central obstacle to physics
    physics_add_box_obstacle(&physics, vec3(0, 2.0f, 0), vec3(5, 4, 5));

    // Create physics vehicles for each entity
    VehicleConfig vehicle_cfg = physics_default_vehicle_config();
    int entity_to_physics[MAX_ENTITIES];  // Maps entity id -> physics vehicle id
    memset(entity_to_physics, -1, sizeof(entity_to_physics));

    for (int i = 0; i < entities.count; i++) {
        Entity* e = &entities.entities[i];
        if (e->active && e->type == ENTITY_VEHICLE) {
            int phys_id = physics_create_vehicle(&physics, e->position, e->rotation_y, &vehicle_cfg);
            entity_to_physics[e->id] = phys_id;
        }
    }

    // Initialize UI renderer
    UIRenderer ui_renderer;
    if (!ui_renderer_init(&ui_renderer)) {
        fprintf(stderr, "Failed to initialize UI renderer\n");
        // Continue anyway, just won't have UI
    }

    // Initialize text renderer
    TextRenderer text_renderer;
    bool has_text = text_renderer_init(&text_renderer, "assets/fonts/Roboto-Bold.ttf", 18.0f);
    if (!has_text) {
        fprintf(stderr, "Failed to initialize text renderer\n");
        // Continue anyway, just won't have text
    }

    // Initialize line renderer for ghost path
    LineRenderer line_renderer;
    bool has_lines = line_renderer_init(&line_renderer);
    if (!has_lines) {
        fprintf(stderr, "Failed to initialize line renderer\n");
    }

    // Light direction (sun from upper-right-front)
    Vec3 light_dir = vec3_normalize(vec3(0.5f, -1.0f, 0.3f));

    // Input state
    InputState input;
    memset(&input, 0, sizeof(InputState));

    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);

    // Planning state
    PlanningState planning = {
        .speed_choice = SPEED_HOLD,
        .selected_phase = 0,
        .current_speed = 0  // Starting from standstill
    };

    // Debug flags
    bool show_cars = true;       // Toggle with 'H' key
    bool debug_ghost = false;    // Toggle with 'G' key for debug output
    bool show_physics_debug = false;  // Toggle with 'P' key for physics shapes
    bool show_help = false;      // Toggle with 'F1' key for help overlay

    // Game mode (F to toggle)
    GameMode game_mode = MODE_TURN_BASED;

    // Physics state for each car (indexed by entity id)
    CarPhysics car_physics[MAX_ENTITIES];
    memset(car_physics, 0, sizeof(car_physics));

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

        // Help overlay toggle
        if (input.keys_pressed[KEY_F1]) {
            show_help = !show_help;
        }

        // Quit on ESC
        if (input.keys_pressed[KEY_ESCAPE]) {
            platform.should_quit = true;
        }

        // Toggle car visibility with H
        if (input.keys_pressed[KEY_H]) {
            show_cars = !show_cars;
            printf("Cars %s\n", show_cars ? "visible" : "hidden");
        }

        // Toggle ghost debug with G
        if (input.keys_pressed[KEY_G]) {
            debug_ghost = !debug_ghost;
            printf("Ghost debug %s\n", debug_ghost ? "ON" : "OFF");
        }

        // Toggle physics debug with P
        if (input.keys_pressed[KEY_P]) {
            show_physics_debug = !show_physics_debug;
            printf("Physics debug %s\n", show_physics_debug ? "ON" : "OFF");
        }

        // Toggle game mode with F
        if (input.keys_pressed[KEY_F]) {
            game_mode = (game_mode == MODE_TURN_BASED) ? MODE_FREESTYLE : MODE_TURN_BASED;
            printf("Game mode: %s\n", game_mode == MODE_FREESTYLE ? "FREESTYLE" : "TURN-BASED");
        }

        // Left click handling (UI buttons first, then 3D picking)
        if (input.mouse_pressed[MOUSE_LEFT] && !input.mouse_captured) {
            float mx = (float)input.mouse_x;
            float my = (float)input.mouse_y;
            bool ui_clicked = false;

            // Speed button rects (must match rendering positions)
            UIRect brake_btn = ui_rect(platform.width - 305, 100, 80, 50);
            UIRect hold_btn = ui_rect(platform.width - 210, 100, 80, 50);
            UIRect accel_btn = ui_rect(platform.width - 115, 100, 80, 50);

            // Check speed button clicks
            if (point_in_rect(mx, my, brake_btn)) {
                planning.speed_choice = SPEED_BRAKE;
                ui_clicked = true;
            } else if (point_in_rect(mx, my, hold_btn)) {
                planning.speed_choice = SPEED_HOLD;
                ui_clicked = true;
            } else if (point_in_rect(mx, my, accel_btn)) {
                planning.speed_choice = SPEED_ACCEL;
                ui_clicked = true;
            }

            // Check phase box clicks
            for (int i = 0; i < 5; i++) {
                UIRect phase_box = ui_rect(platform.width - 305 + i * 58, 200, 52, 80);
                if (point_in_rect(mx, my, phase_box)) {
                    planning.selected_phase = i;
                    ui_clicked = true;
                    break;
                }
            }

            // Check Execute button click
            UIRect execute_btn = ui_rect(platform.width - 315, 305, 300, 50);
            if (point_in_rect(mx, my, execute_btn)) {
                Entity* selected = entity_manager_get_selected(&entities);
                if (selected) {
                    // Calculate end position (same as ghost path)
                    int next_speed = calculate_next_speed(planning.current_speed, planning.speed_choice);
                    float move_dist = calculate_move_distance(next_speed);
                    Vec3 end_pos = calculate_end_position(selected->position, selected->rotation_y, move_dist);

                    // Teleport car to end position
                    selected->position = end_pos;

                    // Update speed for next turn
                    planning.current_speed = next_speed;

                    // Reset to HOLD for next turn planning
                    planning.speed_choice = SPEED_HOLD;

                    printf("Executed turn: %s car moved to (%.1f, %.1f), speed now %d mph\n",
                           selected->team == TEAM_RED ? "Red" : "Blue",
                           selected->position.x, selected->position.z,
                           planning.current_speed);
                } else {
                    printf("No vehicle selected - select a car first!\n");
                }
                ui_clicked = true;
            }

            // If no UI was clicked, do 3D picking
            if (!ui_clicked) {
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
        }

        // Update camera
        camera_update(&camera, &input, dt);

        // Freestyle physics update (ODE-based)
        if (game_mode == MODE_FREESTYLE) {
            Entity* selected = entity_manager_get_selected(&entities);
            if (selected && selected->id < MAX_ENTITIES) {
                int phys_id = entity_to_physics[selected->id];
                if (phys_id >= 0) {
                    // Get input (arrow keys for car control)
                    float throttle = 0.0f;
                    float brake = 0.0f;
                    float steer = 0.0f;

                    if (input.keys[KEY_UP]) throttle = 1.0f;
                    if (input.keys[KEY_DOWN]) brake = 1.0f;
                    if (input.keys[KEY_LEFT]) steer = -1.0f;
                    if (input.keys[KEY_RIGHT]) steer = 1.0f;

                    // Apply controls to physics vehicle
                    physics_vehicle_set_throttle(&physics, phys_id, throttle);
                    physics_vehicle_set_brake(&physics, phys_id, brake);
                    physics_vehicle_set_steering(&physics, phys_id, steer);
                }
            }

            // Step physics simulation
            physics_step(&physics, dt);

            // Sync physics state back to entities
            for (int i = 0; i < entities.count; i++) {
                Entity* e = &entities.entities[i];
                if (!e->active || e->type != ENTITY_VEHICLE) continue;

                int phys_id = entity_to_physics[e->id];
                if (phys_id >= 0) {
                    Vec3 pos;
                    float rot_y;
                    physics_vehicle_get_position(&physics, phys_id, &pos);
                    physics_vehicle_get_rotation(&physics, phys_id, &rot_y);
                    e->position = pos;
                    e->rotation_y = rot_y;

                    // Store velocity for display
                    float speed_ms;
                    physics_vehicle_get_velocity(&physics, phys_id, &speed_ms);
                    if (e->id < MAX_ENTITIES) {
                        car_physics[e->id].velocity = speed_ms;
                    }
                }
            }
        }

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
        if (show_cars) {
            draw_entities(&box_renderer, &entities, &car_mesh);
        }
        box_renderer_end(&box_renderer);

        // Draw ghost path for selected vehicle
        if (has_lines) {
            Entity* selected = entity_manager_get_selected(&entities);
            if (selected) {
                // Calculate predicted movement
                int next_speed = calculate_next_speed(planning.current_speed, planning.speed_choice);
                float move_dist = calculate_move_distance(next_speed);

                // Height for ghost elements (raised above ground for visibility)
                float ghost_y = 0.5f;

                // Start and end positions (at ground level for path calc)
                Vec3 start_ground = vec3(selected->position.x, 0, selected->position.z);
                Vec3 end_ground = calculate_end_position(start_ground, selected->rotation_y, move_dist);

                // Raised positions for rendering
                Vec3 start = vec3(start_ground.x, ghost_y, start_ground.z);
                Vec3 end = vec3(end_ground.x, ghost_y, end_ground.z);

                // Debug output
                if (debug_ghost) {
                    printf("Ghost: speed=%d->%d, dist=%.2f, start=(%.1f,%.1f), end=(%.1f,%.1f), rot=%.2f\n",
                           planning.current_speed, next_speed, move_dist,
                           start.x, start.z, end.x, end.z, selected->rotation_y);
                }

                // Draw the path
                line_renderer_begin(&line_renderer, &view, &projection);

                // Path line (cyan/teal color)
                Vec3 path_color = vec3(0.0f, 0.8f, 0.8f);
                if (move_dist > 0.01f) {
                    line_renderer_draw_line(&line_renderer, start, end, path_color, 0.9f);
                }

                // Circle at start position (always show)
                line_renderer_draw_circle(&line_renderer, start, 1.0f, path_color, 0.8f);

                // Circle at end position (larger, shows where car will be)
                Vec3 end_color = vec3(0.2f, 1.0f, 0.4f);  // Green for destination
                line_renderer_draw_circle(&line_renderer, end, 1.5f, end_color, 0.9f);

                // Draw ghost car outline at end position
                // Simple box outline representing car footprint
                float car_half_len = 2.25f;  // Half of CAR_LENGTH
                float car_half_wid = 1.0f;   // Half of CAR_WIDTH
                float cos_r = cosf(selected->rotation_y);
                float sin_r = sinf(selected->rotation_y);

                // Four corners of car at end position (raised)
                Vec3 corners[5];
                corners[0] = vec3(end.x + (-car_half_len * sin_r - car_half_wid * cos_r),
                                  ghost_y,
                                  end.z + (-car_half_len * cos_r + car_half_wid * sin_r));
                corners[1] = vec3(end.x + (car_half_len * sin_r - car_half_wid * cos_r),
                                  ghost_y,
                                  end.z + (car_half_len * cos_r + car_half_wid * sin_r));
                corners[2] = vec3(end.x + (car_half_len * sin_r + car_half_wid * cos_r),
                                  ghost_y,
                                  end.z + (car_half_len * cos_r - car_half_wid * sin_r));
                corners[3] = vec3(end.x + (-car_half_len * sin_r + car_half_wid * cos_r),
                                  ghost_y,
                                  end.z + (-car_half_len * cos_r - car_half_wid * sin_r));
                corners[4] = corners[0];  // Close the loop

                line_renderer_draw_path(&line_renderer, corners, 5, end_color, 0.9f);

                line_renderer_end(&line_renderer);
            }

            // Physics debug visualization (press P to toggle)
            if (show_physics_debug) {
                line_renderer_begin(&line_renderer, &view, &projection);
                physics_debug_draw(&physics, &line_renderer);
                line_renderer_end(&line_renderer);
            }
        }

        // Draw UI test panels
        ui_renderer_begin(&ui_renderer, platform.width, platform.height);

        // Right side panel (where controls will go)
        ui_draw_panel(&ui_renderer,
            ui_rect(platform.width - 320, 10, 310, platform.height - 20),
            UI_COLOR_PANEL, UI_COLOR_SELECTED, 2.0f, 8.0f);

        // Header bar
        ui_draw_panel(&ui_renderer,
            ui_rect(platform.width - 315, 15, 300, 40),
            UI_COLOR_BG_DARK, UI_COLOR_ACCENT, 1.0f, 4.0f);

        // Speed control section
        ui_draw_panel(&ui_renderer,
            ui_rect(platform.width - 315, 65, 300, 100),
            UI_COLOR_BG_DARK, ui_color(0.3f, 0.3f, 0.4f, 1.0f), 1.0f, 4.0f);

        // Three speed buttons (highlight selected)
        {
            float sel_border = 3.0f;
            float norm_border = 1.0f;
            ui_draw_panel(&ui_renderer,
                ui_rect(platform.width - 305, 100, 80, 50),
                UI_COLOR_DANGER, UI_COLOR_WHITE,
                planning.speed_choice == SPEED_BRAKE ? sel_border : norm_border, 4.0f);
            ui_draw_panel(&ui_renderer,
                ui_rect(platform.width - 210, 100, 80, 50),
                UI_COLOR_SELECTED, UI_COLOR_WHITE,
                planning.speed_choice == SPEED_HOLD ? sel_border : norm_border, 4.0f);
            ui_draw_panel(&ui_renderer,
                ui_rect(platform.width - 115, 100, 80, 50),
                UI_COLOR_SAFE, UI_COLOR_WHITE,
                planning.speed_choice == SPEED_ACCEL ? sel_border : norm_border, 4.0f);
        }

        // Phase boxes section
        ui_draw_panel(&ui_renderer,
            ui_rect(platform.width - 315, 175, 300, 120),
            UI_COLOR_BG_DARK, ui_color(0.3f, 0.3f, 0.4f, 1.0f), 1.0f, 4.0f);

        // 5 phase boxes (highlight selected)
        for (int i = 0; i < 5; i++) {
            bool selected = (i == planning.selected_phase);
            UIColor box_color = selected ? UI_COLOR_CAUTION : UI_COLOR_BG_DARK;
            UIColor border = selected ? UI_COLOR_WHITE : ui_color(0.4f, 0.4f, 0.5f, 1.0f);
            float border_w = selected ? 2.0f : 1.0f;
            ui_draw_panel(&ui_renderer,
                ui_rect(platform.width - 305 + i * 58, 200, 52, 80),
                box_color, border, border_w, 4.0f);
        }

        // Execute button (below phase boxes)
        ui_draw_panel(&ui_renderer,
            ui_rect(platform.width - 315, 305, 300, 50),
            UI_COLOR_ACCENT, UI_COLOR_WHITE, 2.0f, 4.0f);

        // Bottom status bar
        ui_draw_panel(&ui_renderer,
            ui_rect(10, platform.height - 50, platform.width - 340, 40),
            UI_COLOR_PANEL, UI_COLOR_SELECTED, 1.0f, 4.0f);

        ui_renderer_end(&ui_renderer);

        // Draw text labels
        if (has_text) {
            text_renderer_begin(&text_renderer, platform.width, platform.height);

            // Header text (changes based on mode)
            const char* header_text = (game_mode == MODE_FREESTYLE) ? "FREESTYLE MODE" : "TURN PLANNING";
            text_draw_centered(&text_renderer, header_text,
                ui_rect(platform.width - 315, 15, 300, 40), UI_COLOR_WHITE);

            // Speed section label with current speed
            {
                char speed_text[32];
                snprintf(speed_text, sizeof(speed_text), "Current: %d mph", planning.current_speed);
                text_draw(&text_renderer, "SPEED", platform.width - 305, 70, UI_COLOR_WHITE);
                text_draw(&text_renderer, speed_text, platform.width - 200, 70, UI_COLOR_DISABLED);
            }

            // Speed button labels
            text_draw_centered(&text_renderer, "BRAKE",
                ui_rect(platform.width - 305, 100, 80, 50), UI_COLOR_WHITE);
            text_draw_centered(&text_renderer, "HOLD",
                ui_rect(platform.width - 210, 100, 80, 50), UI_COLOR_WHITE);
            text_draw_centered(&text_renderer, "ACCEL",
                ui_rect(platform.width - 115, 100, 80, 50), UI_COLOR_WHITE);

            // Phase section label
            text_draw(&text_renderer, "MANEUVERS", platform.width - 305, 180, UI_COLOR_WHITE);

            // Phase labels
            const char* phase_labels[] = {"P1", "P2", "P3", "P4", "P5"};
            for (int i = 0; i < 5; i++) {
                UIRect box = ui_rect(platform.width - 305 + i * 58, 200, 52, 80);
                text_draw_centered(&text_renderer, phase_labels[i], box, UI_COLOR_WHITE);
            }

            // Execute button label
            text_draw_centered(&text_renderer, "EXECUTE TURN",
                ui_rect(platform.width - 315, 305, 300, 50), UI_COLOR_WHITE);

            // Status bar text with dynamic info
            {
                const char* team_name = "None";
                Entity* sel = entity_manager_get_selected(&entities);
                if (sel) {
                    team_name = sel->team == TEAM_RED ? "Red" :
                                sel->team == TEAM_BLUE ? "Blue" : "Other";
                }

                char status_text[128];
                if (game_mode == MODE_FREESTYLE) {
                    // Freestyle: show real-time velocity
                    float vel = 0.0f;
                    if (sel && sel->id < MAX_ENTITIES) {
                        vel = car_physics[sel->id].velocity;
                    }
                    // Convert to mph-ish (velocity * 2.25 gives nice numbers)
                    int display_mph = (int)(fabsf(vel) * 2.25f);
                    snprintf(status_text, sizeof(status_text),
                        "[F] Mode: FREESTYLE  |  Vehicle: %s  |  Speed: %d mph  |  Arrow keys to drive",
                        team_name, display_mph);
                } else {
                    // Turn-based: show planning info
                    const char* speed_names[] = {"BRAKE", "HOLD", "ACCEL"};
                    snprintf(status_text, sizeof(status_text),
                        "[F] Mode: TURNS  |  Vehicle: %s  |  Speed: %d mph  |  Next: %s  |  Phase: P%d",
                        team_name, planning.current_speed,
                        speed_names[planning.speed_choice], planning.selected_phase + 1);
                }
                text_draw(&text_renderer, status_text, 20, platform.height - 42, UI_COLOR_WHITE);
            }

            text_renderer_end(&text_renderer);
        }

        // Help overlay (renders on top of everything)
        if (show_help) {
            // Semi-transparent background panel - upper left, more transparent
            float help_w = 320.0f;
            float help_h = 480.0f;
            float help_x = 15.0f;
            float help_y = 15.0f;

            ui_renderer_begin(&ui_renderer, platform.width, platform.height);
            ui_draw_panel(&ui_renderer,
                ui_rect(help_x, help_y, help_w, help_h),
                ui_color(0.05f, 0.05f, 0.1f, 0.7f),
                ui_color(0.3f, 0.5f, 0.8f, 0.5f), 1.0f, 6.0f);
            ui_renderer_end(&ui_renderer);

            if (has_text) {
                text_renderer_begin(&text_renderer, platform.width, platform.height);

                float tx = help_x + 15;
                float ty = help_y + 12;
                float line_h = 24.0f;

                text_draw(&text_renderer, "CONTROLS (F1)", tx, ty, UI_COLOR_ACCENT);
                ty += line_h * 1.3f;

                text_draw(&text_renderer, "CAMERA", tx, ty, UI_COLOR_CAUTION);
                ty += line_h;
                text_draw(&text_renderer, "  RMB+drag  Look", tx, ty, UI_COLOR_WHITE);
                ty += line_h;
                text_draw(&text_renderer, "  WASD      Move", tx, ty, UI_COLOR_WHITE);
                ty += line_h;
                text_draw(&text_renderer, "  E/Space   Up", tx, ty, UI_COLOR_WHITE);
                ty += line_h;
                text_draw(&text_renderer, "  Q/Ctrl    Down", tx, ty, UI_COLOR_WHITE);
                ty += line_h;
                text_draw(&text_renderer, "  Shift     Fast", tx, ty, UI_COLOR_WHITE);
                ty += line_h * 1.3f;

                text_draw(&text_renderer, "GAMEPLAY", tx, ty, UI_COLOR_CAUTION);
                ty += line_h;
                text_draw(&text_renderer, "  LMB       Select", tx, ty, UI_COLOR_WHITE);
                ty += line_h;
                text_draw(&text_renderer, "  Arrows    Drive", tx, ty, UI_COLOR_WHITE);
                ty += line_h;
                text_draw(&text_renderer, "  F         Mode", tx, ty, UI_COLOR_WHITE);
                ty += line_h * 1.3f;

                text_draw(&text_renderer, "DEBUG", tx, ty, UI_COLOR_CAUTION);
                ty += line_h;
                text_draw(&text_renderer, "  P         Physics", tx, ty, UI_COLOR_WHITE);
                ty += line_h;
                text_draw(&text_renderer, "  H         Hide cars", tx, ty, UI_COLOR_WHITE);
                ty += line_h;
                text_draw(&text_renderer, "  G         Ghost", tx, ty, UI_COLOR_WHITE);
                ty += line_h * 1.3f;

                text_draw(&text_renderer, "SYSTEM", tx, ty, UI_COLOR_CAUTION);
                ty += line_h;
                text_draw(&text_renderer, "  F11       Fullscreen", tx, ty, UI_COLOR_WHITE);
                ty += line_h;
                text_draw(&text_renderer, "  ESC       Quit", tx, ty, UI_COLOR_WHITE);

                text_renderer_end(&text_renderer);
            }
        }

        // Swap buffers
        platform_swap_buffers(&platform);
    }

    // Cleanup
    physics_destroy(&physics);
    if (has_lines) line_renderer_destroy(&line_renderer);
    if (has_text) text_renderer_destroy(&text_renderer);
    ui_renderer_destroy(&ui_renderer);
    obj_destroy(&car_mesh);
    box_renderer_destroy(&box_renderer);
    floor_destroy(&arena_floor);
    platform_shutdown(&platform);
    printf("Goodbye!\n");

    return 0;
}
