/*
 * tabletop Handling System
 *
 * Tracks Handling Class (HC) and Handling Status (HS) for vehicles.
 * Determines when control rolls are needed and whether they succeed.
 *
 * HC = Base handling ability (from chassis + suspension + tires)
 * HS = Current control state (starts at HC, decreases with maneuvers)
 *
 * When HS goes negative after a maneuver, a control roll is required:
 *   Roll 2d6 + HS >= 7 to maintain control
 *   Failure means crash table lookup
 */

#ifndef HANDLING_H
#define HANDLING_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Result of a control roll
typedef enum {
    CONTROL_SUCCESS,           // Maneuver succeeded, no issues
    CONTROL_ROLL_PASSED,       // Control roll was needed and passed
    CONTROL_ROLL_FAILED,       // Control roll failed - crash table needed
} ControlResult;

// Crash table type (for when control is lost)
typedef enum {
    CRASH_TABLE_NONE,          // No crash
    CRASH_TABLE_1_MANEUVER,    // Crash Table 1 - failed maneuver
    CRASH_TABLE_2_HAZARD,      // Crash Table 2 - hazard event
} CrashTableType;

// Vehicle handling state
typedef struct {
    int handling_class;        // Base HC (calculated from equipment)
    int handling_status;       // Current HS (can go negative)
    int max_hc;                // Maximum HC (can be reduced by tire loss, etc.)

    // Last roll info (for display/debugging)
    int last_roll;             // Last 2d6 roll result
    int last_roll_target;      // What was needed (usually 7)
    ControlResult last_result; // Result of last control check
} VehicleHandling;

// Initialize handling state with calculated HC
void handling_init(VehicleHandling* h, int handling_class);

// Reset HS to HC (call at start of turn)
void handling_reset_turn(VehicleHandling* h);

// Apply a maneuver's difficulty (D value) to handling status
// Returns the control result - whether maneuver succeeded or crash table needed
ControlResult handling_apply_maneuver(VehicleHandling* h, int difficulty);

// Apply a hazard's difficulty to handling status
// Uses Crash Table 2 on failure instead of Table 1
ControlResult handling_apply_hazard(VehicleHandling* h, int difficulty);

// Recover +1 HS (call when driving straight, up to max HC)
void handling_recover(VehicleHandling* h);

// Check if a control roll would be needed for a given difficulty
// (For UI preview - doesn't actually apply the maneuver)
bool handling_would_need_roll(const VehicleHandling* h, int difficulty);

// Calculate total HC from equipment
// chassis_hc_mod: from chassis type (e.g., subcompact +1, van -1)
// suspension_hc: from suspension type (e.g., improved = 2 for cars)
// tire_hc_bonus: from tire type (e.g., radials +1)
int handling_calculate_hc(int chassis_hc_mod, int suspension_hc, int tire_hc_bonus);

// Roll 2d6 (for control rolls and crash tables)
int handling_roll_2d6(void);

// Get string description of control result
const char* handling_result_string(ControlResult result);

#ifdef __cplusplus
}
#endif

#endif // HANDLING_H
