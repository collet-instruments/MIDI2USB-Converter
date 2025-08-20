# GitHub Actions Workflows

This directory contains the CI/CD workflows for the USB MIDI 2.0 Converter project.

## Workflows

### ci.yml
Main CI pipeline that runs on every push and pull request.
- Builds the STM32 firmware
- Runs all unit tests
- Uploads firmware artifacts

### code-quality.yml
Code quality checks:
- Static analysis with cppcheck
- Code formatting verification with clang-format

### test-coverage.yml
Test coverage analysis:
- Runs tests with coverage enabled
- Generates HTML and XML coverage reports
- Displays coverage summary

### release.yml
Automated release creation:
- Triggers on version tags (v*)
- Builds release firmware
- Creates GitHub release with binaries

### local-test.yml
Simple workflow for testing CI configuration:
- Can be triggered manually
- Useful for debugging workflow syntax

## Removed Workflows

### unit-tests.yml
This workflow was removed as it duplicated functionality from:
- `ci.yml` - Handles unit tests and firmware builds
- `code-quality.yml` - Handles code quality checks

## Local Testing

Before pushing changes, you can test the CI locally:

```bash
# Run the local CI test script
./ci/test-ci-locally.sh

# Install optional CI tools
./ci/install-ci-tools.sh
```

## Required Tools

### For Firmware Build:
- arm-none-eabi-gcc
- cmake (>= 3.16)
- ninja or make

### For Code Quality (optional):
- cppcheck
- clang-format
- gcovr (for coverage)

## Workflow Status

You can check the status of workflows in the Actions tab of the GitHub repository.

## Troubleshooting

### Workflow not running?
- Check if the workflow file has correct syntax
- Ensure the trigger conditions are met
- Check if Actions are enabled for the repository

### Build failing?
- Review the CMake configuration
- Check if all submodules are properly initialized
- Verify toolchain installation

### Tests failing?
- Run tests locally first with `cd test && make test`
- Check for platform-specific issues
- Review test output in the Actions log