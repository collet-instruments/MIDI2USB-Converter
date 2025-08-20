# Contributing to MIDI2USB Converter

Thank you for your interest in contributing to the MIDI2USB Converter project! We welcome contributions from the community.

## How to Contribute

### Reporting Issues

- Use the GitHub Issues tab to report bugs or request features
- Check existing issues before creating a new one
- Provide detailed information about the issue:
  - Steps to reproduce
  - Expected behavior
  - Actual behavior
  - Hardware/software environment

### Pull Requests

1. **Fork the Repository**
   - Fork the project to your GitHub account
   - Clone your fork locally

2. **Create a Branch**
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/your-bugfix-name
   ```

3. **Make Your Changes**
   - Follow the existing code style
   - Add unit tests for new functionality
   - Update documentation as needed

4. **Test Your Changes**
   ```bash
   # Run unit tests
   cd test
   make clean && make test
   
   # Run CI tests locally
   ./ci/test-ci-locally.sh
   ```

5. **Commit Your Changes**
   - Use clear, descriptive commit messages
   - Reference issue numbers if applicable
   ```bash
   git commit -m "Add feature: description of your changes (#issue-number)"
   ```

6. **Push and Create Pull Request**
   - Push your branch to your fork
   - Create a pull request to the main repository
   - Describe your changes in the PR description

## Code Style Guidelines

### C/C++ Code
- Follow the existing code formatting
- Use meaningful variable and function names
- Add comments for complex logic
- Keep functions focused and small
- Use the provided `.clang-format` configuration

### Testing
- Write unit tests for new functionality
- Ensure all tests pass before submitting PR
- Maintain or improve code coverage

## Development Environment Setup

### Prerequisites
- ARM GCC toolchain (arm-none-eabi-gcc)
- CMake and Ninja build system
- Unity test framework (included as submodule)
- STM32F401 development board (for hardware testing)

### Building the Project
```bash
# Clone with submodules
git clone --recursive <repository-url>
cd MIDI2USB_Converter

# Build using CMake
cmake --preset Debug
cmake --build build/Debug

# Or use VS Code
.vscode/build.sh "$(pwd)"
```

## Project Structure

- `/Core` - Main application source code
- `/Drivers` - STM32 HAL drivers
- `/test` - Unit tests and mocks
- `/submodules` - External dependencies
- `/cmake` - Build configuration

## Licensing

By contributing to this project, you agree that your contributions will be licensed under the MIT License.

## Questions?

If you have questions about contributing, feel free to:
- Open a GitHub Discussion
- Create an issue with the "question" label
- Check existing documentation in the `/docs` folder

Thank you for contributing to MIDI2USB Converter!