/*
 * ODE Physics Implementation
 * Vehicle physics with independent wheel suspension using Hinge2 joints
 */

#include "ode_physics.h"
#include "../render/line_render.h"
#include <ode/ode.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Contact parameters
#define MAX_CONTACTS 10
#define CONTACT_SURFACE_MU 1.5f       // Friction coefficient
#define CONTACT_SURFACE_SLIP1 0.001f  // Slip parameters
#define CONTACT_SURFACE_SLIP2 0.001f
#define CONTACT_SOFT_ERP 0.5f
#define CONTACT_SOFT_CFM 0.001f

// Static world pointer for collision callback
static PhysicsWorld* g_current_world = NULL;

// Collision callback
static void near_callback(void* data, dGeomID o1, dGeomID o2) {
    (void)data;

    // Get the bodies attached to the geometries
    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);

    // Skip if same body or both static
    if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact)) {
        return;
    }

    // Create contact joints
    dContact contacts[MAX_CONTACTS];
    int num_contacts = dCollide(o1, o2, MAX_CONTACTS, &contacts[0].geom, sizeof(dContact));

    for (int i = 0; i < num_contacts; i++) {
        contacts[i].surface.mode = dContactSlip1 | dContactSlip2 |
                                   dContactSoftERP | dContactSoftCFM |
                                   dContactApprox1;
        contacts[i].surface.mu = CONTACT_SURFACE_MU;
        contacts[i].surface.slip1 = CONTACT_SURFACE_SLIP1;
        contacts[i].surface.slip2 = CONTACT_SURFACE_SLIP2;
        contacts[i].surface.soft_erp = CONTACT_SOFT_ERP;
        contacts[i].surface.soft_cfm = CONTACT_SOFT_CFM;

        dJointID c = dJointCreateContact(g_current_world->world,
                                          g_current_world->contact_group,
                                          &contacts[i]);
        dJointAttach(c, b1, b2);
    }
}

bool physics_init(PhysicsWorld* pw) {
    memset(pw, 0, sizeof(PhysicsWorld));

    // Initialize ODE
    dInitODE();

    // Create world
    pw->world = dWorldCreate();
    dWorldSetGravity(pw->world, 0, -9.81f, 0);
    dWorldSetERP(pw->world, 0.8f);
    dWorldSetCFM(pw->world, 1e-5f);

    // Auto-disable for performance (bodies at rest stop simulating)
    dWorldSetAutoDisableFlag(pw->world, 1);
    dWorldSetAutoDisableLinearThreshold(pw->world, 0.01f);
    dWorldSetAutoDisableAngularThreshold(pw->world, 0.01f);
    dWorldSetAutoDisableSteps(pw->world, 10);

    // Create collision space
    pw->space = dHashSpaceCreate(0);

    // Create contact joint group
    pw->contact_group = dJointGroupCreate(0);

    // Default timestep (60 Hz physics)
    pw->step_size = 1.0f / 60.0f;
    pw->accumulator = 0;

    printf("ODE Physics initialized\n");
    return true;
}

void physics_destroy(PhysicsWorld* pw) {
    // Destroy all vehicles
    for (int i = 0; i < pw->vehicle_count; i++) {
        if (pw->vehicles[i].active) {
            physics_destroy_vehicle(pw, i);
        }
    }

    // Destroy ODE objects
    if (pw->ground) dGeomDestroy(pw->ground);
    dJointGroupDestroy(pw->contact_group);
    dSpaceDestroy(pw->space);
    dWorldDestroy(pw->world);
    dCloseODE();

    printf("ODE Physics destroyed\n");
}

void physics_step(PhysicsWorld* pw, float dt) {
    g_current_world = pw;

    // Accumulate time
    pw->accumulator += dt;

    // Fixed timestep simulation
    while (pw->accumulator >= pw->step_size) {
        // Apply vehicle controls
        for (int i = 0; i < pw->vehicle_count; i++) {
            PhysicsVehicle* v = &pw->vehicles[i];
            if (!v->active) continue;

            // Apply steering to front wheels
            float steer_angle = v->steering * v->config.max_steer_angle;
            dJointSetHinge2Param(v->suspensions[WHEEL_FL], dParamLoStop, steer_angle);
            dJointSetHinge2Param(v->suspensions[WHEEL_FL], dParamHiStop, steer_angle);
            dJointSetHinge2Param(v->suspensions[WHEEL_FR], dParamLoStop, steer_angle);
            dJointSetHinge2Param(v->suspensions[WHEEL_FR], dParamHiStop, steer_angle);

            // Apply motor force to rear wheels (rear-wheel drive)
            float motor_speed = 0.0f;
            float motor_force = 0.0f;

            if (v->throttle > 0.01f) {
                // Accelerate (negative because of wheel axis direction)
                motor_speed = -v->throttle * 30.0f;  // Target angular velocity
                motor_force = v->config.max_motor_force;
            } else if (v->brake > 0.01f) {
                // Brake - apply force to stop wheels
                motor_speed = 0;
                motor_force = v->config.max_brake_force * v->brake;
            }
            // When no input, motor_force = 0 means wheels roll freely

            dJointSetHinge2Param(v->suspensions[WHEEL_RL], dParamVel2, motor_speed);
            dJointSetHinge2Param(v->suspensions[WHEEL_RL], dParamFMax2, motor_force);
            dJointSetHinge2Param(v->suspensions[WHEEL_RR], dParamVel2, motor_speed);
            dJointSetHinge2Param(v->suspensions[WHEEL_RR], dParamFMax2, motor_force);
        }

        // Collision detection
        dSpaceCollide(pw->space, NULL, near_callback);

        // Step simulation
        dWorldQuickStep(pw->world, pw->step_size);

        // Clear contact joints
        dJointGroupEmpty(pw->contact_group);

        pw->accumulator -= pw->step_size;
    }

    // Update wheel states for rendering
    for (int i = 0; i < pw->vehicle_count; i++) {
        PhysicsVehicle* v = &pw->vehicles[i];
        if (!v->active) continue;

        for (int w = 0; w < 4; w++) {
            const dReal* pos = dBodyGetPosition(v->wheels[w]);
            v->wheel_states[w].position = vec3((float)pos[0], (float)pos[1], (float)pos[2]);

            // Get wheel spin from Hinge2 axis 2 angle
            v->wheel_states[w].rotation = (float)dJointGetHinge2Angle2(v->suspensions[w]);

            // Get steering angle for front wheels
            if (w == WHEEL_FL || w == WHEEL_FR) {
                v->wheel_states[w].steer_angle = (float)dJointGetHinge2Angle1(v->suspensions[w]);
            }
        }
    }
}

void physics_set_ground(PhysicsWorld* pw, float y_level) {
    if (pw->ground) {
        dGeomDestroy(pw->ground);
    }
    // Create infinite ground plane at y_level, normal pointing up
    pw->ground = dCreatePlane(pw->space, 0, 1, 0, y_level);
}

void physics_add_box_obstacle(PhysicsWorld* pw, Vec3 pos, Vec3 size) {
    dGeomID box = dCreateBox(pw->space, size.x, size.y, size.z);
    dGeomSetPosition(box, pos.x, pos.y, pos.z);
    // Static obstacle (no body attached)
}

int physics_create_vehicle(PhysicsWorld* pw, Vec3 position, float rotation_y,
                           const VehicleConfig* config) {
    if (pw->vehicle_count >= MAX_PHYSICS_VEHICLES) {
        fprintf(stderr, "Max vehicles reached\n");
        return -1;
    }

    int id = pw->vehicle_count++;
    PhysicsVehicle* v = &pw->vehicles[id];
    memset(v, 0, sizeof(PhysicsVehicle));
    v->id = id;
    v->active = true;
    v->config = *config;

    // Create chassis body
    // Position chassis so wheels touch ground (chassis center above wheel centers)
    v->chassis = dBodyCreate(pw->world);
    float chassis_y = position.y + config->wheel_radius + config->chassis_height * 0.4f;
    dBodySetPosition(v->chassis, position.x, chassis_y, position.z);

    // Set chassis rotation
    dMatrix3 R;
    dRFromAxisAndAngle(R, 0, 1, 0, rotation_y);
    dBodySetRotation(v->chassis, R);

    // Chassis mass
    // Note: Car model has length along Z axis, width along X axis
    dMass mass;
    dMassSetBoxTotal(&mass, config->chassis_mass,
                     config->chassis_width, config->chassis_height, config->chassis_length);
    dBodySetMass(v->chassis, &mass);

    // Disable auto-disable for chassis (we want it always active)
    dBodySetAutoDisableFlag(v->chassis, 0);

    // Chassis collision geometry
    // ODE box: (X, Y, Z) = (width, height, length) to match car model orientation
    v->chassis_geom = dCreateBox(pw->space,
                                  config->chassis_width,
                                  config->chassis_height,
                                  config->chassis_length);
    dGeomSetBody(v->chassis_geom, v->chassis);

    // Wheel positions relative to chassis center
    // Car forward is Z axis, left-right is X axis
    float wz = config->chassis_length * 0.35f;  // Front/back offset (Z axis)
    float wx = config->chassis_width * 0.5f + config->wheel_width * 0.6f;  // Left/right offset (X axis)
    float wy = -config->chassis_height * 0.5f;  // Below chassis center

    Vec3 wheel_offsets[4] = {
        {-wx, wy,  wz},  // Front Left  (+Z = front, -X = left)
        { wx, wy,  wz},  // Front Right (+Z = front, +X = right)
        {-wx, wy, -wz},  // Rear Left   (-Z = rear,  -X = left)
        { wx, wy, -wz},  // Rear Right  (-Z = rear,  +X = right)
    };

    // Create wheels
    for (int w = 0; w < 4; w++) {
        // Wheel body
        v->wheels[w] = dBodyCreate(pw->world);

        // Transform wheel offset by chassis rotation
        float wx_rotated = wheel_offsets[w].x * cosf(rotation_y) - wheel_offsets[w].z * sinf(rotation_y);
        float wz_rotated = wheel_offsets[w].x * sinf(rotation_y) + wheel_offsets[w].z * cosf(rotation_y);

        // Position wheels relative to chassis (not ground)
        float wheel_x = position.x + wx_rotated;
        float wheel_y = chassis_y + wheel_offsets[w].y;  // Relative to chassis center
        float wheel_z = position.z + wz_rotated;

        dBodySetPosition(v->wheels[w], wheel_x, wheel_y, wheel_z);

        // Wheel mass (cylinder)
        dMass wheel_mass;
        dMassSetCylinderTotal(&wheel_mass, config->wheel_mass,
                              3,  // Z-axis aligned cylinder
                              config->wheel_radius, config->wheel_width);
        dBodySetMass(v->wheels[w], &wheel_mass);

        // Disable auto-disable for wheels
        dBodySetAutoDisableFlag(v->wheels[w], 0);

        // Wheel collision (sphere for simplicity, rolls better)
        v->wheel_geoms[w] = dCreateSphere(pw->space, config->wheel_radius);
        dGeomSetBody(v->wheel_geoms[w], v->wheels[w]);

        // Create Hinge2 joint (suspension + steering)
        // Axis 1 = suspension/steering (vertical), Axis 2 = wheel spin (lateral)
        v->suspensions[w] = dJointCreateHinge2(pw->world, 0);
        dJointAttach(v->suspensions[w], v->chassis, v->wheels[w]);
        dJointSetHinge2Anchor(v->suspensions[w], wheel_x, wheel_y, wheel_z);

        // Axis 1: Suspension/steering axis (points up in world space)
        // Using new API: dJointSetHinge2Axes
        // Axis 2: Wheel rotation axis (car's lateral/right direction in world space)
        // Right vector = (cos(rotation_y), 0, -sin(rotation_y))
        float right_x = cosf(rotation_y);
        float right_z = -sinf(rotation_y);

        // Set both axes at once (axis1 = up, axis2 = lateral)
        dReal axis1[3] = {0, 1, 0};  // Up (suspension)
        dReal axis2[3] = {right_x, 0, right_z};  // Lateral (wheel spin)
        dJointSetHinge2Axes(v->suspensions[w], axis1, axis2);

        // Suspension parameters
        dJointSetHinge2Param(v->suspensions[w], dParamSuspensionERP, config->suspension_erp);
        dJointSetHinge2Param(v->suspensions[w], dParamSuspensionCFM, config->suspension_cfm);

        // Steering limits (only front wheels steer)
        if (w == WHEEL_FL || w == WHEEL_FR) {
            dJointSetHinge2Param(v->suspensions[w], dParamLoStop, -config->max_steer_angle);
            dJointSetHinge2Param(v->suspensions[w], dParamHiStop, config->max_steer_angle);
        } else {
            // Rear wheels don't steer
            dJointSetHinge2Param(v->suspensions[w], dParamLoStop, 0);
            dJointSetHinge2Param(v->suspensions[w], dParamHiStop, 0);
        }
    }

    printf("Created physics vehicle %d at (%.1f, %.1f, %.1f)\n",
           id, position.x, position.y, position.z);
    return id;
}

void physics_destroy_vehicle(PhysicsWorld* pw, int vehicle_id) {
    if (vehicle_id < 0 || vehicle_id >= pw->vehicle_count) return;
    PhysicsVehicle* v = &pw->vehicles[vehicle_id];
    if (!v->active) return;

    // Destroy joints
    for (int w = 0; w < 4; w++) {
        if (v->suspensions[w]) dJointDestroy(v->suspensions[w]);
    }

    // Destroy geometries
    if (v->chassis_geom) dGeomDestroy(v->chassis_geom);
    for (int w = 0; w < 4; w++) {
        if (v->wheel_geoms[w]) dGeomDestroy(v->wheel_geoms[w]);
    }

    // Destroy bodies
    if (v->chassis) dBodyDestroy(v->chassis);
    for (int w = 0; w < 4; w++) {
        if (v->wheels[w]) dBodyDestroy(v->wheels[w]);
    }

    v->active = false;
    printf("Destroyed physics vehicle %d\n", vehicle_id);
}

void physics_vehicle_set_steering(PhysicsWorld* pw, int vehicle_id, float steering) {
    if (vehicle_id < 0 || vehicle_id >= pw->vehicle_count) return;
    PhysicsVehicle* v = &pw->vehicles[vehicle_id];
    if (!v->active) return;
    v->steering = steering;
    if (v->steering > 1.0f) v->steering = 1.0f;
    if (v->steering < -1.0f) v->steering = -1.0f;
}

void physics_vehicle_set_throttle(PhysicsWorld* pw, int vehicle_id, float throttle) {
    if (vehicle_id < 0 || vehicle_id >= pw->vehicle_count) return;
    PhysicsVehicle* v = &pw->vehicles[vehicle_id];
    if (!v->active) return;
    v->throttle = throttle;
    if (v->throttle > 1.0f) v->throttle = 1.0f;
    if (v->throttle < 0.0f) v->throttle = 0.0f;
}

void physics_vehicle_set_brake(PhysicsWorld* pw, int vehicle_id, float brake) {
    if (vehicle_id < 0 || vehicle_id >= pw->vehicle_count) return;
    PhysicsVehicle* v = &pw->vehicles[vehicle_id];
    if (!v->active) return;
    v->brake = brake;
    if (v->brake > 1.0f) v->brake = 1.0f;
    if (v->brake < 0.0f) v->brake = 0.0f;
}

void physics_vehicle_get_position(PhysicsWorld* pw, int vehicle_id, Vec3* pos) {
    if (vehicle_id < 0 || vehicle_id >= pw->vehicle_count) return;
    PhysicsVehicle* v = &pw->vehicles[vehicle_id];
    if (!v->active) return;

    const dReal* p = dBodyGetPosition(v->chassis);
    pos->x = (float)p[0];
    pos->y = (float)p[1];
    pos->z = (float)p[2];
}

void physics_vehicle_get_rotation(PhysicsWorld* pw, int vehicle_id, float* rotation_y) {
    if (vehicle_id < 0 || vehicle_id >= pw->vehicle_count) return;
    PhysicsVehicle* v = &pw->vehicles[vehicle_id];
    if (!v->active) return;

    const dReal* R = dBodyGetRotation(v->chassis);
    // Extract Y rotation from rotation matrix
    // R is 3x4 matrix in row-major: R[0..3] = row0, R[4..7] = row1, R[8..11] = row2
    *rotation_y = atan2f((float)R[2], (float)R[0]);
}

void physics_vehicle_get_velocity(PhysicsWorld* pw, int vehicle_id, float* speed_ms) {
    if (vehicle_id < 0 || vehicle_id >= pw->vehicle_count) return;
    PhysicsVehicle* v = &pw->vehicles[vehicle_id];
    if (!v->active) return;

    const dReal* vel = dBodyGetLinearVel(v->chassis);
    // Speed in m/s (magnitude of horizontal velocity)
    *speed_ms = sqrtf((float)(vel[0]*vel[0] + vel[2]*vel[2]));
}

void physics_vehicle_get_wheel_states(PhysicsWorld* pw, int vehicle_id, WheelState* wheels) {
    if (vehicle_id < 0 || vehicle_id >= pw->vehicle_count) return;
    PhysicsVehicle* v = &pw->vehicles[vehicle_id];
    if (!v->active) return;

    memcpy(wheels, v->wheel_states, sizeof(WheelState) * 4);
}

VehicleConfig physics_default_vehicle_config(void) {
    VehicleConfig cfg = {
        .chassis_mass = 1200.0f,     // 1200 kg
        .chassis_length = 4.5f,      // 4.5 meters
        .chassis_width = 2.0f,       // 2 meters
        .chassis_height = 1.2f,      // 1.2 meters

        .wheel_mass = 20.0f,         // 20 kg per wheel
        .wheel_radius = 0.4f,        // 0.4 meter radius
        .wheel_width = 0.25f,        // 0.25 meter wide

        .suspension_erp = 0.4f,      // Moderate stiffness
        .suspension_cfm = 0.02f,     // Some softness
        .suspension_travel = 0.3f,   // 30cm travel

        .max_steer_angle = 0.5f,     // ~30 degrees
        .max_motor_force = 5000.0f,  // 5000 N
        .max_brake_force = 8000.0f,  // 8000 N
    };
    return cfg;
}

// Helper to draw a wireframe box at a given position/rotation
static void draw_box_wireframe(LineRenderer* lr, const dReal* pos, const dReal* R,
                                float lx, float ly, float lz, Vec3 color) {
    // Half sizes
    float hx = lx * 0.5f;
    float hy = ly * 0.5f;
    float hz = lz * 0.5f;

    // 8 corners in local space
    float corners_local[8][3] = {
        {-hx, -hy, -hz}, { hx, -hy, -hz}, { hx, -hy,  hz}, {-hx, -hy,  hz},
        {-hx,  hy, -hz}, { hx,  hy, -hz}, { hx,  hy,  hz}, {-hx,  hy,  hz}
    };

    // Transform to world space
    Vec3 corners[8];
    for (int i = 0; i < 8; i++) {
        float lx_i = corners_local[i][0];
        float ly_i = corners_local[i][1];
        float lz_i = corners_local[i][2];
        // R is 3x4 row-major rotation matrix
        corners[i].x = (float)pos[0] + R[0]*lx_i + R[1]*ly_i + R[2]*lz_i;
        corners[i].y = (float)pos[1] + R[4]*lx_i + R[5]*ly_i + R[6]*lz_i;
        corners[i].z = (float)pos[2] + R[8]*lx_i + R[9]*ly_i + R[10]*lz_i;
    }

    // Draw 12 edges
    // Bottom face
    line_renderer_draw_line(lr, corners[0], corners[1], color, 1.0f);
    line_renderer_draw_line(lr, corners[1], corners[2], color, 1.0f);
    line_renderer_draw_line(lr, corners[2], corners[3], color, 1.0f);
    line_renderer_draw_line(lr, corners[3], corners[0], color, 1.0f);
    // Top face
    line_renderer_draw_line(lr, corners[4], corners[5], color, 1.0f);
    line_renderer_draw_line(lr, corners[5], corners[6], color, 1.0f);
    line_renderer_draw_line(lr, corners[6], corners[7], color, 1.0f);
    line_renderer_draw_line(lr, corners[7], corners[4], color, 1.0f);
    // Vertical edges
    line_renderer_draw_line(lr, corners[0], corners[4], color, 1.0f);
    line_renderer_draw_line(lr, corners[1], corners[5], color, 1.0f);
    line_renderer_draw_line(lr, corners[2], corners[6], color, 1.0f);
    line_renderer_draw_line(lr, corners[3], corners[7], color, 1.0f);
}

void physics_debug_draw(PhysicsWorld* pw, LineRenderer* lr) {
    Vec3 chassis_color = vec3(1.0f, 1.0f, 0.0f);  // Yellow for chassis
    Vec3 wheel_color = vec3(0.0f, 1.0f, 1.0f);    // Cyan for wheels
    Vec3 ground_color = vec3(0.3f, 0.8f, 0.3f);   // Green for ground

    // Draw ground plane indicator (just a few lines at y=0)
    for (float x = -30; x <= 30; x += 10) {
        line_renderer_draw_line(lr, vec3(x, 0.01f, -30), vec3(x, 0.01f, 30), ground_color, 0.3f);
    }
    for (float z = -30; z <= 30; z += 10) {
        line_renderer_draw_line(lr, vec3(-30, 0.01f, z), vec3(30, 0.01f, z), ground_color, 0.3f);
    }

    // Draw vehicles
    for (int i = 0; i < pw->vehicle_count; i++) {
        PhysicsVehicle* v = &pw->vehicles[i];
        if (!v->active) continue;

        // Draw chassis box (X=width, Y=height, Z=length to match car model)
        const dReal* chassis_pos = dBodyGetPosition(v->chassis);
        const dReal* chassis_R = dBodyGetRotation(v->chassis);
        draw_box_wireframe(lr, chassis_pos, chassis_R,
                          v->config.chassis_width,
                          v->config.chassis_height,
                          v->config.chassis_length,
                          chassis_color);

        // Draw wheels as circles
        for (int w = 0; w < 4; w++) {
            const dReal* wheel_pos = dBodyGetPosition(v->wheels[w]);
            Vec3 center = vec3((float)wheel_pos[0], (float)wheel_pos[1], (float)wheel_pos[2]);
            line_renderer_draw_circle(lr, center, v->config.wheel_radius, wheel_color, 1.0f);
        }
    }
}
