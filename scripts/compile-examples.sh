#!/bin/bash

################################################################################
#
#  SensythingCore - Example Compilation Test Script
#
#  This script uses arduino-cli to compile all example sketches and verify
#  that the header rename from SensythingES3.h to SensythingCore.h is working
#  correctly across all boards and communication types.
#
#  Prerequisites:
#    - arduino-cli installed and in PATH
#    - ESP32 board package installed: arduino-cli board listall | grep esp32
#    - SensythingCore library installed in Arduino libraries folder
#    - Protocentral_FDC1004 library installed
#    - Protocentral_AFE44xx library installed
#    - WebSockets library installed
#
#  Usage:
#    ./compile-examples.sh [--verbose] [--stop-on-error] [--board BOARD]
#
#  Options:
#    --verbose          Show full compiler output (default: errors only)
#    --stop-on-error    Exit immediately on first compilation error
#    --board BOARD      Only compile for specific board (default: esp32:esp32:esp32s3)
#
#  Exit Codes:
#    0 - All sketches compiled successfully
#    1 - One or more sketches failed to compile
#    2 - Prerequisites not met (arduino-cli not found, missing libraries, etc.)
#
#  Examples:
#    ./compile-examples.sh                    # Run all tests, show errors only
#    ./compile-examples.sh --verbose          # Show full compiler output
#    ./compile-examples.sh --stop-on-error    # Exit on first failure
#
################################################################################

set -o pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BOARD_FQBN="${BOARD_FQBN:-esp32:esp32:esp32s3}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
EXAMPLES_DIR="$PROJECT_ROOT/examples"
BUILD_DIR="/tmp/sensything-build-$(date +%s)"
VERBOSE=false
STOP_ON_ERROR=false

# Counters
TOTAL_SKETCHES=0
COMPILED_OK=0
COMPILED_FAIL=0
SKIPPED=0

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --verbose)
            VERBOSE=true
            shift
            ;;
        --stop-on-error)
            STOP_ON_ERROR=true
            shift
            ;;
        --board)
            BOARD_FQBN="$2"
            shift 2
            ;;
        --help)
            head -n 50 "$0" | tail -n 45
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 2
            ;;
    esac
done

# ============================================================================
# UTILITY FUNCTIONS
# ============================================================================

print_header() {
    echo ""
    echo -e "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════════════${NC}"
    echo ""
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

# ============================================================================
# PREREQUISITE CHECKS
# ============================================================================

check_prerequisites() {
    print_header "Checking Prerequisites"
    
    # Check if arduino-cli is installed
    if ! command -v arduino-cli &> /dev/null; then
        print_error "arduino-cli not found. Please install it first."
        echo "  Installation: https://arduino.cc/en/software"
        return 1
    fi
    print_success "arduino-cli found: $(arduino-cli version)"
    
    # Check if ESP32 board is installed
    if ! arduino-cli board listall | grep -q "esp32:esp32"; then
        print_error "ESP32 board package not installed"
        echo "  Install with: arduino-cli core install esp32:esp32@latest"
        return 1
    fi
    print_success "ESP32 board package installed"
    
    # Verify board FQBN
    if ! arduino-cli board details -b "$BOARD_FQBN" &> /dev/null; then
        print_error "Board not found: $BOARD_FQBN"
        echo "  Available ESP32 boards:"
        arduino-cli board listall | grep esp32 | head -5
        return 1
    fi
    print_success "Board verified: $BOARD_FQBN"
    
    # Check for required libraries (warning only, as they might be in different locations)
    local required_libs=("Protocentral_FDC1004" "WebSockets")
    local missing_libs=0
    
    for lib in "${required_libs[@]}"; do
        if ! arduino-cli lib list | grep -q "$lib"; then
            print_warning "Library not found: $lib"
            missing_libs=$((missing_libs + 1))
        else
            print_success "Library found: $lib"
        fi
    done
    
    if [ $missing_libs -gt 0 ]; then
        print_warning "Some libraries may not be installed. Compilation may fail for certain sketches."
        return 0  # Still continue, as some sketches might still work
    fi
    
    return 0
}

# ============================================================================
# BUILD DIRECTORY SETUP
# ============================================================================

setup_build_directory() {
    print_header "Setting up Build Directory"
    
    if [ -d "$BUILD_DIR" ]; then
        print_info "Cleaning existing build directory: $BUILD_DIR"
        rm -rf "$BUILD_DIR"
    fi
    
    mkdir -p "$BUILD_DIR"
    print_success "Build directory ready: $BUILD_DIR"
}

# ============================================================================
# SKETCH COMPILATION
# ============================================================================

compile_sketch() {
    local sketch_path="$1"
    local sketch_name=$(basename "$(dirname "$sketch_path")")
    local category=$(basename "$(dirname "$(dirname "$sketch_path")")")
    local section=$(basename "$(dirname "$(dirname "$(dirname "$sketch_path")")")")
    
    echo -n "  Compiling: $section/$category/$sketch_name ... "
    
    # Create build directory for this sketch
    local sketch_build_dir="$BUILD_DIR/${section}/${category}/${sketch_name}"
    mkdir -p "$sketch_build_dir"
    
    # Compile the sketch
    local compile_output
    if compile_output=$(arduino-cli compile --fqbn "$BOARD_FQBN" \
                                             --build-path "$sketch_build_dir" \
                                             "$sketch_path" 2>&1); then
        print_success "OK"
        COMPILED_OK=$((COMPILED_OK + 1))
        return 0
    else
        print_error "FAILED"
        COMPILED_FAIL=$((COMPILED_FAIL + 1))
        
        # Show compilation output
        if [ "$VERBOSE" = true ]; then
            echo ""
            echo "    Error output:"
            echo "$compile_output" | sed 's/^/    /'
            echo ""
        else
            # Show just the key error lines
            echo "    Summary:"
            echo "$compile_output" | grep -E "error:|undefined reference|fatal" | sed 's/^/    /'
        fi
        
        if [ "$STOP_ON_ERROR" = true ]; then
            echo ""
            echo "    Full output:"
            echo "$compile_output"
            return 1
        fi
        
        return 1
    fi
}

# ============================================================================
# MAIN COMPILATION LOOP
# ============================================================================

compile_all_sketches() {
    print_header "Compiling All Sketches"
    print_info "Board: $BOARD_FQBN"
    print_info "Build directory: $BUILD_DIR"
    echo ""
    
    # Find all .ino files
    local sketches=()
    while IFS= read -r -d '' sketch; do
        sketches+=("$sketch")
    done < <(find "$EXAMPLES_DIR" -name "*.ino" -print0 | sort -z)
    
    if [ ${#sketches[@]} -eq 0 ]; then
        print_error "No sketches found in $EXAMPLES_DIR"
        return 1
    fi
    
    print_info "Found ${#sketches[@]} sketches to compile"
    echo ""
    
    # Compile each sketch
    TOTAL_SKETCHES=${#sketches[@]}
    for sketch in "${sketches[@]}"; do
        if ! compile_sketch "$sketch"; then
            if [ "$STOP_ON_ERROR" = true ]; then
                return 1
            fi
        fi
    done
    
    return 0
}

# ============================================================================
# SUMMARY AND REPORTING
# ============================================================================

print_summary() {
    print_header "Compilation Summary"
    
    echo "Total sketches found:     $TOTAL_SKETCHES"
    echo -e "Successfully compiled:    ${GREEN}$COMPILED_OK${NC}"
    
    if [ $COMPILED_FAIL -gt 0 ]; then
        echo -e "Failed to compile:        ${RED}$COMPILED_FAIL${NC}"
        return 1
    else
        echo -e "Failed to compile:        ${GREEN}0${NC}"
    fi
    
    if [ $SKIPPED -gt 0 ]; then
        echo -e "Skipped:                  ${YELLOW}$SKIPPED${NC}"
    fi
    
    echo ""
    echo "Build artifacts: $BUILD_DIR"
    echo ""
    
    if [ $COMPILED_FAIL -eq 0 ]; then
        print_success "All sketches compiled successfully! ✨"
        return 0
    else
        print_error "$COMPILED_FAIL sketch(es) failed to compile"
        echo ""
        echo "To see full error details, run with --verbose flag:"
        echo "  $0 --verbose"
        return 1
    fi
}

# ============================================================================
# MAIN EXECUTION
# ============================================================================

main() {
    print_header "SensythingCore Example Compilation Test"
    echo "Testing header rename: SensythingES3.h → SensythingCore.h"
    echo ""
    
    # Check prerequisites
    if ! check_prerequisites; then
        exit 2
    fi
    
    echo ""
    
    # Setup build directory
    setup_build_directory
    
    echo ""
    
    # Compile all sketches
    if ! compile_all_sketches; then
        if [ "$STOP_ON_ERROR" = false ]; then
            echo ""  # Spacing before summary
        fi
    fi
    
    echo ""
    
    # Print summary
    if print_summary; then
        exit 0
    else
        exit 1
    fi
}

# Run main function
main "$@"
