/*
 * Car Wars Maneuver System - Kinematic Path Animation
 *
 * Executes maneuvers using kinematic interpolation along a calculated path.
 * Vehicle is switched to kinematic mode during maneuver, then back to dynamic.
 *
 * Flow:
 * 1. Player requests maneuver while paused
 * 2. System validates speed requirements
 * 3. System calculates target position/heading from Car Wars rules
 * 4. Vehicle switches to KINEMATIC mode
 * 5. Each frame: interpolate position/heading along path, use MoveKinematic
 * 6. When path complete: switch back to DYNAMIC, set velocity to match
 * 7. Control returns to player
 *
 * Interruption: If collision/hazard detected mid-maneuver, immediately
 * switch to dynamic mode and let physics handle the chaos.
 */

#ifndef MANEUVER_H
#define MANEUVER_H

#include <stdbool.h>
#include "../math/vec3.h"

#ifdef __cplusplus
extern "C" {
#endif

// Car Wars scale: 1" = 15 feet = 4.572 meters
#define CW_INCH_TO_METERS 4.572f
#define CW_QUARTER_INCH   (CW_INCH_TO_METERS * 0.25f)  // 1.143m
#define CW_HALF_INCH      (CW_INCH_TO_METERS * 0.5f)   // 2.286m
#define CW_THREE_QUARTER  (CW_INCH_TO_METERS * 0.75f)  // 3.429m

// Maneuver types
typedef enum {
    MANEUVER_NONE = 0,

    // Basic maneuvers (Phase 2)
    MANEUVER_STRAIGHT,        // D0: no lateral, no heading change - just move forward
    MANEUVER_DRIFT,           // D1: 1/4" lateral, keep heading
    MANEUVER_STEEP_DRIFT,     // D3: 1/2" lateral, keep heading
    MANEUVER_BEND,            // D1-D6: turn with heading change

    // Advanced maneuvers (Phase 3+)
    MANEUVER_SWERVE,          // Drift + opposite bend
    MANEUVER_CONTROLLED_SKID, // D+1 to D+4: powerslide

    // Special maneuvers (Phase 4)
    MANEUVER_PIVOT,           // D0: 5 mph only, pivot around rear corner
    MANEUVER_T_STOP,          // D1/10mph: emergency 90° brake
    MANEUVER_BOOTLEGGER,      // D7: 20-35 mph, J-turn 180°

    MANEUVER_COUNT
} ManeuverType;

// Direction for lateral maneuvers
typedef enum {
    MANEUVER_LEFT = -1,
    MANEUVER_RIGHT = 1
} ManeuverDirection;

// Autopilot state
typedef enum {
    AUTOPILOT_IDLE,           // No maneuver active
    AUTOPILOT_STARTING,       // Just started, initializing
    AUTOPILOT_EXECUTING,      // Steering toward target
    AUTOPILOT_COMPLETING,     // Near target, settling
    AUTOPILOT_CORRECTING,     // Smoothly correcting heading (animation phase)
    AUTOPILOT_FINISHED,       // Done, returning control
    AUTOPILOT_FAILED          // Timeout or physics failure
} AutopilotState;

// Maneuver request (what the player wants to do)
typedef struct {
    ManeuverType type;
    ManeuverDirection direction;  // LEFT or RIGHT
    int bend_angle;               // For BEND: degrees (15, 30, 45, 60, 75, 90)
    int skid_distance;            // For CONTROLLED_SKID: 1-4 (quarter inches)
} ManeuverRequest;

// Pose for kinematic interpolation
typedef struct {
    Vec3 position;
    float heading;      // Radians
} ManeuverPose;

// Maximum phases in a turn (5 for 50+ mph)
#define MAX_TURN_PHASES 5

// Single phase within a multi-phase turn
typedef struct {
    ManeuverRequest request;      // What maneuver for this phase
    float start_time;             // When this phase starts (0.0 to 1.0)
    float end_time;               // When this phase ends (0.0 to 1.0)

    // Calculated path for this phase
    Vec3 start_position;
    float start_heading;
    Vec3 target_position;
    float target_heading;

    // Arc path parameters (for BEND maneuvers)
    bool is_arc_path;
    float arc_radius;
    Vec3 arc_center;
    float arc_angle;              // Signed angle to sweep
} TurnPhase;

// Autopilot controller state
typedef struct {
    AutopilotState state;
    ManeuverRequest request;      // Current phase's request (for single-phase compat)

    // Start state (captured when maneuver begins)
    Vec3 start_position;
    float start_heading;
    float start_speed_ms;

    // Target state (calculated from Car Wars rules)
    Vec3 target_position;
    float target_heading;         // Radians

    // Arc path parameters (for BEND maneuvers) - current phase
    bool is_arc_path;             // True if following arc instead of linear
    float arc_radius;             // Radius of turn circle (meters)
    Vec3 arc_center;              // Center of turn circle (world coords)
    float arc_angle;              // Total angle to sweep (radians, signed)

    // Timing
    float elapsed;                // Seconds since turn started
    float duration;               // Total turn duration (always 1.0s)
    float progress;               // 0.0 to 1.0 normalized time

    // Current interpolated pose (updated each frame)
    ManeuverPose current_pose;

    // Debug info
    float lateral_displacement;   // Current lateral offset from start
    float forward_displacement;   // Current forward offset from start

    // Multi-phase turn support
    int num_phases;               // Number of phases in this turn (1-5)
    int current_phase;            // Which phase we're currently executing (0-based)
    TurnPhase phases[MAX_TURN_PHASES];  // Phase data array
} ManeuverAutopilot;

// Validate if a maneuver can be performed at current speed
// Returns true if allowed, false if not (with reason in out_reason if provided)
bool maneuver_validate(ManeuverType type, float speed_ms, const char** out_reason);

// Calculate the difficulty (D value) for a maneuver
int maneuver_get_difficulty(ManeuverType type, ManeuverDirection dir, int param);

// Start a single-phase maneuver - calculates path and activates kinematic animation
// Returns false if maneuver not allowed at current speed
bool maneuver_start(ManeuverAutopilot* ap,
                    const ManeuverRequest* request,
                    Vec3 current_pos,
                    float current_heading,
                    float current_speed_ms);

// Start a multi-phase turn - executes all phases as one continuous 1.0s animation
// phase_indices: which Car Wars phases are active (e.g., {1, 3} for P2 and P4 at 20 mph)
// requests: maneuver request for each active phase
// num_phases: number of active phases (1-5)
// Returns false if any phase validation fails
bool maneuver_start_turn(ManeuverAutopilot* ap,
                         const int* phase_indices,
                         const ManeuverRequest* requests,
                         int num_phases,
                         Vec3 current_pos,
                         float current_heading,
                         float current_speed_ms);

// Update autopilot - called each physics frame
// Calculates interpolated pose for this frame
// Sets *out_complete to true when maneuver is done
// Returns the current pose to move the vehicle to
ManeuverPose maneuver_update(ManeuverAutopilot* ap,
                             float dt,
                             bool* out_complete);

// Cancel a maneuver in progress
void maneuver_cancel(ManeuverAutopilot* ap);

// Check if autopilot is active (vehicle should be kinematic)
bool maneuver_is_active(const ManeuverAutopilot* ap);

// Get the exit velocity (direction and speed for when switching back to dynamic)
Vec3 maneuver_get_exit_velocity(const ManeuverAutopilot* ap);

// Get maneuver name for display
const char* maneuver_get_name(ManeuverType type);

// Get maneuver status string for display
const char* maneuver_get_status(const ManeuverAutopilot* ap);

#ifdef __cplusplus
}
#endif

#endif // MANEUVER_H
