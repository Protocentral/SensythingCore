#!/bin/bash

################################################################################
#
#  SensythingCore - Compile & Upload OX WiFi App
#
#  Compiles and uploads the OX_WiFi_App example to a connected
#  Sensything OX (ESP32-S3) board. Uses the huge_app partition scheme
#  required by the full embedded web dashboard.
#
#  Prerequisites:
#    - arduino-cli installed and in PATH
#    - ESP32 board package installed
#    - Required libraries: WebSockets, ProtoCentral AFE4490
#    - Sensything OX connected via USB
#
#  Usage:
#    ./upload-ox-wifi-app.sh [--port PORT] [--compile-only] [--verbose]
#
#  Options:
#    --port PORT        Serial port (default: auto-detect)
#    --compile-only     Compile without uploading
#    --verbose          Show full compiler output
#
################################################################################

set -euo pipefail

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
BOARD_FQBN="esp32:esp32:esp32s3"
BOARD_OPTIONS="PartitionScheme=huge_app,PSRAM=enabled,CDCOnBoot=cdc,USBMode=hwcdc"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SKETCH_DIR="$PROJECT_ROOT/examples/03.Applications/OX_WiFi_App"
PORT=""
COMPILE_ONLY=false
VERBOSE=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --port)
            PORT="$2"
            shift 2
            ;;
        --compile-only)
            COMPILE_ONLY=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            head -n 30 "$0" | tail -n 25
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

print_step() {
    echo -e "${BLUE}=> $1${NC}"
}

# Check prerequisites
if ! command -v arduino-cli &> /dev/null; then
    echo -e "${RED}Error: arduino-cli not found. Install from https://arduino.cc/en/software${NC}"
    exit 2
fi

if [ ! -f "$SKETCH_DIR/OX_WiFi_App.ino" ]; then
    echo -e "${RED}Error: Sketch not found at $SKETCH_DIR/OX_WiFi_App.ino${NC}"
    exit 2
fi

echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}  Sensything OX - WiFi App Compile & Upload${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
echo ""

# Compile
print_step "Compiling OX_WiFi_App..."
echo "  Board:   $BOARD_FQBN"
echo "  Options: $BOARD_OPTIONS"
echo ""

COMPILE_ARGS=(
    compile
    -b "$BOARD_FQBN"
    --board-options "$BOARD_OPTIONS"
    --build-path "$PROJECT_ROOT/.build/ox_wifi"
)

if [ "$VERBOSE" = true ]; then
    COMPILE_ARGS+=(-v)
fi

COMPILE_ARGS+=("$SKETCH_DIR")

if arduino-cli "${COMPILE_ARGS[@]}"; then
    echo ""
    echo -e "${GREEN}Compilation successful${NC}"
else
    echo ""
    echo -e "${RED}Compilation failed${NC}"
    exit 1
fi

# Upload
if [ "$COMPILE_ONLY" = true ]; then
    echo -e "${YELLOW}Skipping upload (--compile-only)${NC}"
    exit 0
fi

echo ""
print_step "Uploading to board..."

UPLOAD_ARGS=(
    upload
    -b "$BOARD_FQBN"
    --board-options "$BOARD_OPTIONS"
    --input-dir "$PROJECT_ROOT/.build/ox_wifi"
)

if [ -n "$PORT" ]; then
    UPLOAD_ARGS+=(-p "$PORT")
    echo "  Port: $PORT"
else
    echo "  Port: auto-detect"
fi

if [ "$VERBOSE" = true ]; then
    UPLOAD_ARGS+=(-v)
fi

UPLOAD_ARGS+=("$SKETCH_DIR")

if arduino-cli "${UPLOAD_ARGS[@]}"; then
    echo ""
    echo -e "${GREEN}Upload successful${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Connect to WiFi AP 'sensything' (password: sensything)"
    echo "  2. Open http://192.168.4.1 in your browser"
    echo ""
else
    echo ""
    echo -e "${RED}Upload failed${NC}"
    echo "  - Check that the board is connected via USB"
    echo "  - Try specifying the port: $0 --port /dev/ttyACM0"
    echo "  - Hold BOOT button during upload if needed"
    exit 1
fi
