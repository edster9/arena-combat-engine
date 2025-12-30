# Proposal: Maneuver Smoothing System

**Status:** Proposed
**Date:** 2025-12-29
**Priority:** Low (quality-of-life enhancement)

---

## Problem Statement

At higher speeds, Car Wars phase durations become very short:

| Speed | Phases | Duration per Phase |
|-------|--------|-------------------|
| 10 mph | 1 (P3) | 1.0s |
| 20 mph | 2 (P2, P4) | 0.5s each |
| 30 mph | 3 (P1, P3, P5) | 0.33s each |
| 50 mph | 5 (all) | 0.2s each |

A drift maneuver at 50 mph completes in 0.2 seconds - too fast to look natural. The lateral displacement is physically correct, but the visual appears jerky.

---

## Proposed Solutions

### 1. Maneuver Damping (Optional Toggle)

**Concept:** Use empty (STRAIGHT) phase slots as "damping zones" to spread a maneuver's motion over more time.

**How it works:**
- When enabled, a maneuver declared in one phase can "bleed" into adjacent empty phases
- The total displacement remains identical (same endpoint)
- Motion uses ease-in/ease-out curves across the extended duration
- Only affects phases declared as STRAIGHT (no maneuver)

**Example - 50 mph with Drift at P3:**

Without damping:
```
P1: STRAIGHT (0.2s) → P2: STRAIGHT (0.2s) → P3: DRIFT (0.2s - jerky!) → P4: STRAIGHT (0.2s) → P5: STRAIGHT (0.2s)
```

With damping enabled:
```
P1: ease-in    → P2: accelerate → P3: DRIFT peak → P4: decelerate → P5: ease-out
    (0.2s)          (0.2s)           (0.2s)           (0.2s)           (0.2s)
                         └──── 1.0s total smooth motion ────┘
```

**Constraints:**
- Only damps into phases declared STRAIGHT
- Cannot damp across a phase with another maneuver
- Damping stops at turn boundaries

**Implementation notes:**
- Global toggle: `g_maneuver_damping = true/false`
- Modify interpolation in `maneuver_update()` to use extended duration
- Calculate "influence zones" before/after the declared phase
- Use sine-based easing for natural acceleration/deceleration

---

### 2. Maneuver Blending (Advanced Enhancement)

**Concept:** When multiple maneuvers are declared in consecutive phases, blend them into one continuous fluid motion instead of discrete steps.

**Example - 20 mph with Drift at P2, Bend at P4:**

Without blending:
```
P2: DRIFT LEFT (0.5s) → [brief pause] → P4: BEND RIGHT 30° (0.5s)
```

With blending enabled:
```
P2+P4: Combined smooth S-curve motion (1.0s total)
       - Lateral drift left while beginning turn setup
       - Seamless transition into right bend
       - Car flows through both maneuvers as one motion
```

**Benefits:**
- Eliminates visual "stutter" between consecutive maneuvers
- Creates cinematic racing feel for replays
- Maintains Car Wars rule accuracy (same endpoints)

**Complexity:**
- Requires path planning that combines multiple maneuver geometries
- Must handle opposing directions (drift left + bend right)
- May need bezier/spline interpolation instead of linear/arc

**When NOT to blend:**
- Non-consecutive phases (P1 maneuver, P3 empty, P5 maneuver)
- Player explicitly disables blending
- Crash/hazard interrupts between phases

---

## User Controls

Proposed settings (in options menu or config):

```
[Maneuver Animation]
  [ ] Maneuver Damping    - Smooth single maneuvers using empty phases
  [ ] Maneuver Blending   - Combine consecutive maneuvers into fluid motion
```

Both off = strict frame-accurate Car Wars timing (purist mode)
Both on = maximum cinematic smoothness (replay mode)

---

## Implementation Phases

### Phase A: Continuous Turn Execution (CURRENT WORK)
- All phases in a turn execute as one continuous 1.0s animation
- No pause between phase boundaries
- Phase endpoints are waypoints
- **This is the foundation for damping/blending**

### Phase B: Maneuver Damping
- Add toggle and damping zone calculation
- Modify interpolation to use easing curves
- Test with various speed/maneuver combinations

### Phase C: Maneuver Blending
- Path planning for combined maneuvers
- Spline-based interpolation
- Handle direction conflicts gracefully

---

## Open Questions

1. Should damping be automatic based on speed, or always user-controlled?
2. How much should damping extend? Full available empty phases, or limited?
3. For blending, what interpolation method? Catmull-Rom splines? Bezier?
4. Should these affect AI vehicles too, or only player-controlled?

---

## References

- [maneuver.cpp](../client/src/game/maneuver.cpp) - Current kinematic animation system
- [maneuver.h](../client/src/game/maneuver.h) - ManeuverAutopilot structure
- [CONFIG_GUIDE.md](CONFIG_GUIDE.md) - Car Wars phase timing rules
