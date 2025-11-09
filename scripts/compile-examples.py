#!/usr/bin/env python3

################################################################################
#
#  SensythingCore - Example Compilation Test Script (Python)
#
#  This script uses arduino-cli to compile all example sketches and verify
#  that the header rename from SensythingES3.h to SensythingCore.h is working
#  correctly across all boards and communication types.
#
#  This Python version provides cross-platform compatibility (Windows/Mac/Linux).
#
#  Prerequisites:
#    - Python 3.7+
#    - arduino-cli installed and in PATH
#    - ESP32 board package installed
#    - SensythingCore library installed
#    - Protocentral_FDC1004 library installed
#    - Protocentral_AFE44xx library installed
#    - WebSockets library installed
#
#  Usage:
#    python3 compile-examples.py [--verbose] [--stop-on-error] [--board BOARD]
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
#    python3 compile-examples.py                    # Run all tests, show errors only
#    python3 compile-examples.py --verbose          # Show full compiler output
#    python3 compile-examples.py --stop-on-error    # Exit on first failure
#
################################################################################

import sys
import os
import subprocess
import argparse
import shutil
import tempfile
from pathlib import Path
from typing import List, Tuple, Optional
from datetime import datetime
from enum import Enum

# Python 3.7+ compatibility
if sys.version_info < (3, 7):
    print("Error: Python 3.7 or higher required")
    sys.exit(2)


class Color(Enum):
    """ANSI color codes for terminal output"""
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color


def colored(text: str, color: Color) -> str:
    """Return colored text for terminal output"""
    # Don't add colors if output is being piped
    if not sys.stdout.isatty():
        return text
    return f"{color.value}{text}{Color.NC.value}"


def print_header(text: str) -> None:
    """Print a formatted header"""
    print()
    print(colored("═" * 67, Color.BLUE))
    print(colored(text, Color.BLUE))
    print(colored("═" * 67, Color.BLUE))
    print()


def print_success(text: str) -> None:
    """Print success message"""
    print(colored(f"✓ {text}", Color.GREEN))


def print_error(text: str) -> None:
    """Print error message"""
    print(colored(f"✗ {text}", Color.RED))


def print_warning(text: str) -> None:
    """Print warning message"""
    print(colored(f"⚠ {text}", Color.YELLOW))


def print_info(text: str) -> None:
    """Print info message"""
    print(colored(f"ℹ {text}", Color.BLUE))


class CompilationTest:
    """Main test runner class"""

    def __init__(self, board_fqbn: str, verbose: bool = False, stop_on_error: bool = False):
        self.board_fqbn = board_fqbn
        self.verbose = verbose
        self.stop_on_error = stop_on_error
        
        # Determine project root (script is in scripts/ folder)
        self.script_dir = Path(__file__).parent.resolve()
        self.project_root = self.script_dir.parent
        self.examples_dir = self.project_root / "examples"
        self.build_dir = Path(tempfile.gettempdir()) / f"sensything-build-{int(datetime.now().timestamp())}"
        
        # Statistics
        self.total_sketches = 0
        self.compiled_ok = 0
        self.compiled_fail = 0
        self.failed_sketches: List[str] = []

    def check_prerequisites(self) -> bool:
        """Check if all prerequisites are installed"""
        print_header("Checking Prerequisites")
        
        # Check arduino-cli
        if shutil.which("arduino-cli") is None:
            print_error("arduino-cli not found. Please install it first.")
            print("  Installation: https://arduino.cc/en/software")
            return False
        
        try:
            result = subprocess.run(
                ["arduino-cli", "version"],
                capture_output=True,
                text=True,
                timeout=5
            )
            version_line = result.stdout.strip().split('\n')[0]
            print_success(f"arduino-cli found: {version_line}")
        except Exception as e:
            print_error(f"Failed to get arduino-cli version: {e}")
            return False
        
        # Check ESP32 board
        try:
            result = subprocess.run(
                ["arduino-cli", "board", "listall"],
                capture_output=True,
                text=True,
                timeout=10
            )
            if "esp32:esp32" not in result.stdout:
                print_error("ESP32 board package not installed")
                print("  Install with: arduino-cli core install esp32:esp32@latest")
                return False
            print_success("ESP32 board package installed")
        except Exception as e:
            print_error(f"Failed to list boards: {e}")
            return False
        
        # Verify specific board FQBN
        try:
            result = subprocess.run(
                ["arduino-cli", "board", "details", "-b", self.board_fqbn],
                capture_output=True,
                text=True,
                timeout=5
            )
            if result.returncode != 0:
                print_error(f"Board not found: {self.board_fqbn}")
                print("  Available ESP32 boards:")
                boards_result = subprocess.run(
                    ["arduino-cli", "board", "listall"],
                    capture_output=True,
                    text=True
                )
                for line in boards_result.stdout.split('\n'):
                    if 'esp32' in line.lower():
                        print(f"    {line}")
                        break
                return False
            print_success(f"Board verified: {self.board_fqbn}")
        except Exception as e:
            print_error(f"Failed to verify board: {e}")
            return False
        
        # Check for required libraries (warning only)
        required_libs = ["Protocentral_FDC1004", "WebSockets"]
        missing_libs = 0
        
        try:
            result = subprocess.run(
                ["arduino-cli", "lib", "list"],
                capture_output=True,
                text=True,
                timeout=10
            )
            lib_output = result.stdout
            
            for lib in required_libs:
                if lib in lib_output:
                    print_success(f"Library found: {lib}")
                else:
                    print_warning(f"Library not found: {lib}")
                    missing_libs += 1
        except Exception as e:
            print_warning(f"Could not check libraries: {e}")
        
        if missing_libs > 0:
            print_warning("Some libraries may not be installed. Compilation may fail for certain sketches.")
        
        return True

    def setup_build_directory(self) -> bool:
        """Setup build directory"""
        print_header("Setting up Build Directory")
        
        try:
            if self.build_dir.exists():
                print_info(f"Cleaning existing build directory: {self.build_dir}")
                shutil.rmtree(self.build_dir)
            
            self.build_dir.mkdir(parents=True, exist_ok=True)
            print_success(f"Build directory ready: {self.build_dir}")
            return True
        except Exception as e:
            print_error(f"Failed to setup build directory: {e}")
            return False

    def find_sketches(self) -> List[Path]:
        """Find all .ino sketches in examples directory"""
        if not self.examples_dir.exists():
            print_error(f"Examples directory not found: {self.examples_dir}")
            return []
        
        sketches = sorted(self.examples_dir.glob("**/*.ino"))
        return sketches

    def compile_sketch(self, sketch_path: Path) -> bool:
        """Compile a single sketch"""
        # Extract hierarchy for display
        relative_path = sketch_path.relative_to(self.examples_dir)
        parts = relative_path.parts[:-1]  # All parts except the .ino filename
        sketch_name = sketch_path.stem
        
        # Build path display
        path_display = "/".join(parts) + "/" + sketch_name
        
        print(f"  Compiling: {path_display} ... ", end="", flush=True)
        
        # Create build directory for this sketch
        sketch_build_dir = self.build_dir / "/".join(parts) / sketch_name
        sketch_build_dir.mkdir(parents=True, exist_ok=True)
        
        try:
            # Run compilation
            result = subprocess.run(
                [
                    "arduino-cli", "compile",
                    "--fqbn", self.board_fqbn,
                    "--build-path", str(sketch_build_dir),
                    str(sketch_path)
                ],
                capture_output=True,
                text=True,
                timeout=120
            )
            
            if result.returncode == 0:
                print_success("OK")
                self.compiled_ok += 1
                return True
            else:
                print_error("FAILED")
                self.compiled_fail += 1
                self.failed_sketches.append(str(relative_path))
                
                # Show error output
                if self.verbose:
                    print()
                    print("    Error output:")
                    for line in result.stderr.split('\n'):
                        if line.strip():
                            print(f"    {line}")
                    if result.stdout:
                        for line in result.stdout.split('\n'):
                            if 'error' in line.lower() or 'undefined' in line.lower():
                                print(f"    {line}")
                    print()
                else:
                    # Show just errors
                    print("    Summary:")
                    error_shown = False
                    for line in (result.stderr + result.stdout).split('\n'):
                        if 'error:' in line or 'undefined reference' in line or 'fatal' in line:
                            print(f"    {line}")
                            error_shown = True
                    if not error_shown and result.stderr:
                        # Show first few lines of error
                        first_error = result.stderr.split('\n')[0]
                        if first_error.strip():
                            print(f"    {first_error}")
                
                if self.stop_on_error:
                    print()
                    print("    Full output:")
                    print(result.stderr)
                    if result.stdout:
                        print(result.stdout)
                    return False
                
                return False
                
        except subprocess.TimeoutExpired:
            print_error("TIMEOUT")
            self.compiled_fail += 1
            self.failed_sketches.append(str(relative_path))
            print_error("    Compilation timed out after 120 seconds")
            return not self.stop_on_error
        except Exception as e:
            print_error(f"ERROR: {e}")
            self.compiled_fail += 1
            self.failed_sketches.append(str(relative_path))
            return not self.stop_on_error

    def compile_all_sketches(self) -> bool:
        """Compile all sketches"""
        print_header("Compiling All Sketches")
        print_info(f"Board: {self.board_fqbn}")
        print_info(f"Build directory: {self.build_dir}")
        print()
        
        sketches = self.find_sketches()
        
        if not sketches:
            print_error(f"No sketches found in {self.examples_dir}")
            return False
        
        self.total_sketches = len(sketches)
        print_info(f"Found {self.total_sketches} sketches to compile")
        print()
        
        success = True
        for sketch in sketches:
            if not self.compile_sketch(sketch):
                if self.stop_on_error:
                    return False
                success = False
        
        return success

    def print_summary(self) -> bool:
        """Print compilation summary"""
        print_header("Compilation Summary")
        
        print(f"Total sketches found:     {self.total_sketches}")
        print(f"Successfully compiled:    {colored(str(self.compiled_ok), Color.GREEN)}")
        
        if self.compiled_fail > 0:
            print(f"Failed to compile:        {colored(str(self.compiled_fail), Color.RED)}")
        else:
            print(f"Failed to compile:        {colored('0', Color.GREEN)}")
        
        print()
        print(f"Build artifacts: {self.build_dir}")
        print()
        
        if self.compiled_fail == 0:
            print_success("All sketches compiled successfully! ✨")
            return True
        else:
            print_error(f"{self.compiled_fail} sketch(es) failed to compile:")
            for sketch in self.failed_sketches:
                print(f"  - {sketch}")
            print()
            print("To see full error details, run with --verbose flag:")
            print("  python3 compile-examples.py --verbose")
            return False

    def run(self) -> int:
        """Run the complete test suite"""
        print_header("SensythingCore Example Compilation Test")
        print("Testing header rename: SensythingES3.h → SensythingCore.h")
        print()
        
        # Check prerequisites
        if not self.check_prerequisites():
            return 2
        
        print()
        
        # Setup build directory
        if not self.setup_build_directory():
            return 2
        
        print()
        
        # Compile all sketches
        if not self.compile_all_sketches():
            if not self.stop_on_error:
                print()
        
        print()
        
        # Print summary
        if self.print_summary():
            return 0
        else:
            return 1


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="Compile all SensythingCore example sketches using arduino-cli",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 compile-examples.py                    # Run all tests, show errors only
  python3 compile-examples.py --verbose          # Show full compiler output
  python3 compile-examples.py --stop-on-error    # Exit on first failure

Exit codes:
  0 - All sketches compiled successfully
  1 - One or more sketches failed to compile
  2 - Prerequisites not met
        """
    )
    
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Show full compiler output"
    )
    parser.add_argument(
        "--stop-on-error",
        action="store_true",
        help="Exit immediately on first compilation error"
    )
    parser.add_argument(
        "--board",
        default="esp32:esp32:esp32s3",
        help="Board FQBN (default: esp32:esp32:esp32s3)"
    )
    
    args = parser.parse_args()
    
    # Run test suite
    tester = CompilationTest(
        board_fqbn=args.board,
        verbose=args.verbose,
        stop_on_error=args.stop_on_error
    )
    
    sys.exit(tester.run())


if __name__ == "__main__":
    main()
