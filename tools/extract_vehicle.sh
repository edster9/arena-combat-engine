#!/bin/bash
# Extract body and wheel meshes from a combined vehicle OBJ file
#
# Usage: ./tools/extract_vehicle.sh <input.obj> <body_output.obj> <wheel_output.obj>
#
# Example:
#   ./tools/extract_vehicle.sh assets/models/vehicles/sedan.obj \
#       assets/models/chassis/compact_body.obj \
#       assets/models/wheels/wheel_standard.obj

BLENDER="/mnt/c/Program Files/Blender Foundation/Blender 5.0/blender.exe"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Default paths if none provided
INPUT="${1:-$PROJECT_ROOT/assets/models/vehicles/sedan.obj}"
BODY_OUTPUT="${2:-$PROJECT_ROOT/assets/models/chassis/compact_body.obj}"
WHEEL_OUTPUT="${3:-$PROJECT_ROOT/assets/models/wheels/wheel_standard.obj}"

# Convert to absolute paths if relative
[[ "$INPUT" != /* ]] && INPUT="$PROJECT_ROOT/$INPUT"
[[ "$BODY_OUTPUT" != /* ]] && BODY_OUTPUT="$PROJECT_ROOT/$BODY_OUTPUT"
[[ "$WHEEL_OUTPUT" != /* ]] && WHEEL_OUTPUT="$PROJECT_ROOT/$WHEEL_OUTPUT"

# Convert WSL paths to Windows paths for Blender
SCRIPT_WIN=$(wslpath -w "$SCRIPT_DIR/blender_extract_vehicle_parts.py")
INPUT_WIN=$(wslpath -w "$INPUT")
BODY_WIN=$(wslpath -w "$BODY_OUTPUT")
WHEEL_WIN=$(wslpath -w "$WHEEL_OUTPUT")

echo "=== Vehicle Part Extractor ==="
echo "Input:  $INPUT"
echo "Body:   $BODY_OUTPUT"
echo "Wheel:  $WHEEL_OUTPUT"
echo ""

"$BLENDER" --background --python "$SCRIPT_WIN" -- \
    "$INPUT_WIN" "$BODY_WIN" "$WHEEL_WIN"