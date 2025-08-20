#!/bin/bash

# Local CI test script
# This script simulates the GitHub Actions CI environment locally

set -e

echo "=== Local CI Test ==="
echo "Testing the CI pipeline locally before pushing..."
echo

# Color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓ $2${NC}"
    else
        echo -e "${RED}✗ $2${NC}"
        exit 1
    fi
}

# Check if running from project root
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: This script must be run from the project root directory${NC}"
    exit 1
fi

echo "1. Checking build tools..."
# Check for required tools
if command -v ninja >/dev/null 2>&1; then
    print_status 0 "Ninja build system is installed"
else
    print_status 1 "Ninja build system is installed"
fi

if command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    print_status 0 "ARM toolchain is installed"
else
    echo -e "${YELLOW}⚠ ARM toolchain not found in PATH${NC}"
    echo "  Checking for STM32 bundled toolchain..."
    if [ -d "$HOME/Library/Application Support/stm32cube/bundles/gnu-tools-for-stm32" ]; then
        echo -e "${GREEN}✓ STM32 bundled toolchain found${NC}"
    else
        print_status 1 "ARM toolchain is installed"
    fi
fi

echo
echo "2. Testing firmware build..."
# Use the project's build script
if [ -f ".vscode/build.sh" ]; then
    echo "Using .vscode/build.sh..."
    bash .vscode/build.sh "$PWD"
    print_status $? "Firmware build"
else
    echo -e "${RED}Error: .vscode/build.sh not found${NC}"
    exit 1
fi

echo
echo "3. Testing unit tests..."
cd test

# Clean
make clean >/dev/null 2>&1

# Build tests
make
print_status $? "Test build"

# Run tests
make test
print_status $? "Test execution"

cd ..

echo
echo "4. Checking code quality tools..."
# Check for optional tools
if command -v cppcheck >/dev/null 2>&1; then
    echo -e "${GREEN}✓ cppcheck is installed${NC}"
    # Run basic cppcheck
    cppcheck --enable=warning --suppress=missingInclude --error-exitcode=1 \
        -I Core/Inc \
        Core/Src/mode_manager.c >/dev/null 2>&1
    print_status $? "cppcheck basic check"
else
    echo -e "${YELLOW}⚠ cppcheck is not installed (optional)${NC}"
fi

if command -v clang-format >/dev/null 2>&1; then
    echo -e "${GREEN}✓ clang-format is installed${NC}"
    # Check if .clang-format exists
    if [ -f ".clang-format" ]; then
        # Test format check on one file
        clang-format --dry-run --Werror Core/Src/mode_manager.c >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ Code formatting check passed${NC}"
        else
            echo -e "${YELLOW}⚠ Code formatting issues found (non-critical)${NC}"
        fi
    fi
else
    echo -e "${YELLOW}⚠ clang-format is not installed (optional)${NC}"
fi

echo
echo "5. Validating GitHub Actions workflows..."
# Basic YAML syntax check
for workflow in .github/workflows/*.yml; do
    if [ -f "$workflow" ]; then
        # Just check if file is readable and not empty
        if [ -s "$workflow" ]; then
            echo -e "${GREEN}✓ $(basename $workflow) exists and is not empty${NC}"
        else
            echo -e "${RED}✗ $(basename $workflow) is empty${NC}"
        fi
    fi
done

echo
echo "=== Local CI Test Complete ==="
echo -e "${GREEN}All required checks passed!${NC}"
echo
echo "You can now push your changes with confidence."
echo "The GitHub Actions CI will run these same checks in the cloud."
echo
echo "To test with coverage locally:"
echo "  cd Test"
echo "  make clean && make COVERAGE=1 && make test"
echo
echo "To install optional tools:"
echo "  # Ubuntu/Debian:"
echo "  sudo apt-get install cppcheck clang-format gcovr"
echo "  # macOS:"
echo "  brew install cppcheck clang-format gcovr"