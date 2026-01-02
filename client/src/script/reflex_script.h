/*
 * Reflex Script Engine
 * LuaJIT-based scripting for vehicle control (ABS, traction control, AI)
 * Uses Sol3 for C++ bindings
 */

#ifndef REFLEX_SCRIPT_H
#define REFLEX_SCRIPT_H

#include <stdbool.h>
#include "../math/vec3.h"
#include "../game/config_loader.h"
#include "../physics/jolt_physics.h"

#define MAX_SCRIPT_WHEELS 8
#define MAX_SCRIPT_AXLES 4

// Per-wheel telemetry for scripts
typedef struct {
    int index;              // Wheel index in physics array
    Vec3 position;          // World position
    float rotation;         // Wheel spin angle (radians)
    float angular_velocity; // Wheel spin rate (rad/s) - for slip calculation
    float radius;           // Wheel radius (m) - for linear speed calculation
    float steer_angle;      // Current steering angle
    float suspension;       // Suspension compression (0=extended, 1=compressed)
    bool in_contact;        // Is wheel touching ground?

    // Semantic info
    int side;               // WHEEL_SIDE_LEFT, RIGHT, CENTER
    int axle_index;         // Which axle this wheel belongs to
} ScriptWheelState;

// Per-axle telemetry for scripts
typedef struct {
    int index;              // Axle index
    int position;           // AXLE_POSITION_FRONT, REAR, MIDDLE
    bool has_handbrake;     // Handbrake affects this axle
    bool is_steering;       // This axle can steer
    bool is_driven;         // This axle is powered
    int wheel_count;        // Number of wheels on this axle
    int wheel_indices[4];   // Indices into wheels array
} ScriptAxleState;

// Engine/drivetrain telemetry
typedef struct {
    float rpm;              // Current engine RPM
    float rpm_max;          // Redline RPM
    float rpm_idle;         // Idle RPM
    int gear;               // Current gear (0=neutral, -1=reverse, 1+=forward)
    float throttle;         // Current throttle input (0-1)
} ScriptEngineState;

// Complete vehicle telemetry (read-only inputs for scripts)
typedef struct {
    // Position and orientation
    Vec3 position;          // World position
    float heading;          // Heading in radians (0=+Z, CCW positive)
    float heading_deg;      // Heading in degrees for convenience
    Vec3 velocity;          // Velocity vector (m/s)
    float speed_ms;         // Speed magnitude (m/s)
    float speed_mph;        // Speed in mph for convenience
    float yaw_rate;         // Angular velocity around Y axis (rad/s)
    float lateral_velocity; // Sideways speed (m/s) - for drift detection

    // Handling
    int handling_class;     // Base HC
    int handling_status;    // Current HS (damaged from maneuvers)

    // Wheel and axle data
    ScriptWheelState wheels[MAX_SCRIPT_WHEELS];
    int wheel_count;
    ScriptAxleState axles[MAX_SCRIPT_AXLES];
    int axle_count;

    // Engine state
    ScriptEngineState engine;

    // Preprocessed helpers for common queries
    int front_wheel_indices[4];
    int front_wheel_count;
    int rear_wheel_indices[4];
    int rear_wheel_count;
    int left_wheel_indices[4];
    int left_wheel_count;
    int right_wheel_indices[4];
    int right_wheel_count;
    int handbrake_wheel_indices[4];
    int handbrake_wheel_count;
} ScriptTelemetry;

// Vehicle control outputs (written by scripts)
typedef struct {
    float steering;         // -1 (left) to 1 (right)
    float throttle;         // 0 to 1
    float brake;            // 0 to 1 (all wheels)
    float handbrake;        // 0 to 1 (handbrake axles only)

    // Per-wheel brake control (for ABS)
    float wheel_brake[MAX_SCRIPT_WHEELS];
    bool use_per_wheel_brake;

    // Flags
    bool controls_modified; // True if script modified any controls
} ScriptControls;

// Script instance (one per vehicle that has a script loaded)
typedef struct ScriptInstance ScriptInstance;

// Script engine (singleton, manages all script instances)
typedef struct ReflexScriptEngine ReflexScriptEngine;

#ifdef __cplusplus
extern "C" {
#endif

// Engine lifecycle
// Creates engine and loads master.lua orchestrator
ReflexScriptEngine* reflex_create(void);
void reflex_destroy(ReflexScriptEngine* engine);

// Attach a script to a vehicle (creates isolated script instance)
// script_path: path to the vehicle's script (e.g., "scripts/freestyle_assist.lua")
// config_keys/values: configuration options passed to script as 'config' table
// Returns true on success
bool reflex_attach_script(ReflexScriptEngine* engine,
                          int vehicle_id,
                          const char* script_path,
                          const char* config_keys[],
                          float config_values[],
                          int config_count);

// Detach script from a vehicle
void reflex_detach_script(ReflexScriptEngine* engine, int vehicle_id);

// Update a vehicle's script
// Gathers telemetry, calls script update(), applies control outputs
void reflex_update_vehicle(ReflexScriptEngine* engine,
                           PhysicsWorld* pw,
                           int vehicle_id,
                           float dt);

// Reload all scripts (hot reload for development)
// Calls master.reload_all() which reloads all script files from disk
// Returns number of scripts reloaded, or -1 on error
int reflex_reload_all_scripts(ReflexScriptEngine* engine);

// Apply controls to physics vehicle (called internally, exposed for testing)
void reflex_apply_controls(PhysicsWorld* pw,
                           int vehicle_id,
                           const ScriptControls* controls);

#ifdef __cplusplus
}
#endif

#endif // REFLEX_SCRIPT_H