/*
 * tabletop Handling System Implementation
 */

#include "handling.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Track if RNG has been seeded
static bool rng_seeded = false;

// Roll 2d6
int handling_roll_2d6(void) {
    if (!rng_seeded) {
        srand((unsigned int)time(NULL));
        rng_seeded = true;
    }
    int d1 = (rand() % 6) + 1;
    int d2 = (rand() % 6) + 1;
    return d1 + d2;
}

// Initialize handling state
void handling_init(VehicleHandling* h, int handling_class) {
    h->handling_class = handling_class;
    h->max_hc = handling_class;
    h->handling_status = handling_class;
    h->last_roll = 0;
    h->last_roll_target = 7;
    h->last_result = CONTROL_SUCCESS;
}

// Reset HS to HC at start of turn
void handling_reset_turn(VehicleHandling* h) {
    h->handling_status = h->handling_class;
    h->last_result = CONTROL_SUCCESS;
}

// Internal: perform control roll and return result
static ControlResult do_control_roll(VehicleHandling* h, int target) {
    h->last_roll_target = target;
    h->last_roll = handling_roll_2d6();

    int total = h->last_roll + h->handling_status;

    printf("[Handling] Control roll: 2d6(%d) + HS(%d) = %d vs target %d\n",
           h->last_roll, h->handling_status, total, target);

    if (total >= target) {
        h->last_result = CONTROL_ROLL_PASSED;
        printf("[Handling] Control roll PASSED\n");
        return CONTROL_ROLL_PASSED;
    } else {
        h->last_result = CONTROL_ROLL_FAILED;
        printf("[Handling] Control roll FAILED - Crash Table 1 lookup needed\n");
        return CONTROL_ROLL_FAILED;
    }
}

// Apply maneuver difficulty to handling status
ControlResult handling_apply_maneuver(VehicleHandling* h, int difficulty) {
    if (difficulty <= 0) {
        // D0 maneuvers (like Pivot) don't affect handling
        h->last_result = CONTROL_SUCCESS;
        return CONTROL_SUCCESS;
    }

    int old_hs = h->handling_status;
    h->handling_status -= difficulty;

    printf("[Handling] Maneuver D%d: HS %d -> %d\n", difficulty, old_hs, h->handling_status);

    // Control roll needed when HS goes negative
    if (h->handling_status < 0) {
        return do_control_roll(h, 7);
    }

    h->last_result = CONTROL_SUCCESS;
    return CONTROL_SUCCESS;
}

// Apply hazard difficulty (uses Crash Table 2 on failure)
ControlResult handling_apply_hazard(VehicleHandling* h, int difficulty) {
    if (difficulty <= 0) {
        h->last_result = CONTROL_SUCCESS;
        return CONTROL_SUCCESS;
    }

    int old_hs = h->handling_status;
    h->handling_status -= difficulty;

    printf("[Handling] Hazard D%d: HS %d -> %d\n", difficulty, old_hs, h->handling_status);

    // Control roll needed when HS goes negative
    if (h->handling_status < 0) {
        ControlResult result = do_control_roll(h, 7);
        if (result == CONTROL_ROLL_FAILED) {
            printf("[Handling] Hazard crash - Crash Table 2 lookup needed\n");
        }
        return result;
    }

    h->last_result = CONTROL_SUCCESS;
    return CONTROL_SUCCESS;
}

// Recover +1 HS when driving straight
void handling_recover(VehicleHandling* h) {
    if (h->handling_status < h->handling_class) {
        h->handling_status++;
        printf("[Handling] Recovery: HS -> %d (max %d)\n",
               h->handling_status, h->handling_class);
    }
}

// Check if control roll would be needed (for UI preview)
bool handling_would_need_roll(const VehicleHandling* h, int difficulty) {
    return (h->handling_status - difficulty) < 0;
}

// Calculate total HC from equipment components
int handling_calculate_hc(int chassis_hc_mod, int suspension_hc, int tire_hc_bonus) {
    int total = chassis_hc_mod + suspension_hc + tire_hc_bonus;
    printf("[Handling] HC calculation: chassis(%d) + suspension(%d) + tires(%d) = %d\n",
           chassis_hc_mod, suspension_hc, tire_hc_bonus, total);
    return total;
}

// Get string description of result
const char* handling_result_string(ControlResult result) {
    switch (result) {
        case CONTROL_SUCCESS:      return "Success";
        case CONTROL_ROLL_PASSED:  return "Control Roll Passed";
        case CONTROL_ROLL_FAILED:  return "CRASH TABLE";
        default:                   return "Unknown";
    }
}
