#!/bin/bash

# Install CI tools for local testing

echo "=== Installing CI Tools ==="
echo

# Detect OS
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    echo "Detected macOS"
    echo "Installing tools with Homebrew..."
    
    # Check if Homebrew is installed
    if ! command -v brew >/dev/null 2>&1; then
        echo "Error: Homebrew is not installed"
        echo "Please install Homebrew first: https://brew.sh"
        exit 1
    fi
    
    # Install tools
    brew install cppcheck clang-format gcovr
    
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    echo "Detected Linux"
    echo "Installing tools with apt-get..."
    
    # Update package list
    sudo apt-get update
    
    # Install tools
    sudo apt-get install -y cppcheck clang-format gcovr
    
else
    echo "Unsupported OS: $OSTYPE"
    echo "Please install the following tools manually:"
    echo "- cppcheck"
    echo "- clang-format"
    echo "- gcovr"
    exit 1
fi

echo
echo "=== Installation Complete ==="
echo
echo "Installed tools:"
command -v cppcheck >/dev/null 2>&1 && echo "✓ cppcheck: $(cppcheck --version | head -n1)"
command -v clang-format >/dev/null 2>&1 && echo "✓ clang-format: $(clang-format --version | head -n1)"
command -v gcovr >/dev/null 2>&1 && echo "✓ gcovr: $(gcovr --version | head -n1)"