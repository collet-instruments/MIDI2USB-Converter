# Test Makefile for MIDI2USB-Converter

CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -g -DTESTING=1
CXXFLAGS = $(CFLAGS) -std=c++14

# Coverage flags
ifeq ($(COVERAGE),1)
CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LDFLAGS += --coverage
endif

# Include paths (mock first to override Core headers when TESTING=1)
INCLUDES = -I./mock \
           -I./include \
           -I../submodules/Unity/src \
           -I../submodules/AM_MIDI2.0Lib/include \
           -I../Core/Inc

# Unity source
UNITY_SRC = ../submodules/Unity/src/unity.c

# Mock source
MOCK_SRC = ./mock/mock_freertos.c

# Test sources (excluding test_usb_descriptors.c and test_midi_stubs.c)
TEST_SRCS = $(filter-out src/test_usb_descriptors.c src/test_midi_stubs.c, $(wildcard src/*.c))
TEST_EXES = $(patsubst src/%.c,build/%,$(TEST_SRCS))

# Build directory
BUILD_DIR = build

.PHONY: all clean test

all: $(BUILD_DIR) $(TEST_EXES)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build test executables
$(BUILD_DIR)/%: src/%.c $(UNITY_SRC) $(MOCK_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(UNITY_SRC) $(MOCK_SRC) $(LDFLAGS) -o $@

# Special rule for test_midi_common that needs to link with Core source
$(BUILD_DIR)/test_midi_common: src/test_midi_common.c $(UNITY_SRC) $(MOCK_SRC) ../Core/Src/midi_common.c
	$(CC) $(CFLAGS) $(INCLUDES) -c ../Core/Src/midi_common.c -o $(BUILD_DIR)/midi_common.o
	$(CC) $(CFLAGS) $(INCLUDES) $< $(BUILD_DIR)/midi_common.o $(UNITY_SRC) $(MOCK_SRC) $(LDFLAGS) -o $@


# Special rule for test_ump_task that uses the actual ump_task.c source with GetUmpWordCount
$(BUILD_DIR)/test_ump_task: src/test_ump_task.c $(UNITY_SRC) ./mock/ump_task_stubs.c $(MOCK_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c ../Core/Src/ump_task.c -o $(BUILD_DIR)/ump_task.o
	$(CC) $(CFLAGS) $(INCLUDES) $< $(BUILD_DIR)/ump_task.o ./mock/ump_task_stubs.c $(UNITY_SRC) $(MOCK_SRC) $(LDFLAGS) -o $@

# Special rule for test_ump_discovery that uses the actual ump_discovery.c source
$(BUILD_DIR)/test_ump_discovery: src/test_ump_discovery.c $(UNITY_SRC) $(MOCK_SRC) ../Core/Src/ump_discovery.c ./mock/ump_discovery_mocks.c
	$(CC) $(CFLAGS) $(INCLUDES) -c ../Core/Src/ump_discovery.c -o $(BUILD_DIR)/ump_discovery.o
	$(CC) $(CFLAGS) $(INCLUDES) $< $(BUILD_DIR)/ump_discovery.o ./mock/ump_discovery_mocks.c $(UNITY_SRC) $(MOCK_SRC) $(LDFLAGS) -o $@

# Special rule for test_usb_strings that needs to link with Core source and UMP mocks
$(BUILD_DIR)/test_usb_strings: src/test_usb_strings.c $(UNITY_SRC) ../Core/Src/usb_descriptors.c ./mock/ump_mocks.c
	$(CC) $(CFLAGS) $(INCLUDES) -c ../Core/Src/usb_descriptors.c -o $(BUILD_DIR)/usb_descriptors.o
	$(CC) $(CFLAGS) $(INCLUDES) -c ./mock/ump_mocks.c -o $(BUILD_DIR)/ump_mocks.o
	$(CC) $(CFLAGS) $(INCLUDES) $< $(BUILD_DIR)/usb_descriptors.o $(BUILD_DIR)/ump_mocks.o $(UNITY_SRC) $(LDFLAGS) -o $@

# Run all tests
test: all
	@echo "Running all tests..."
	@failed=0; \
	for test in $(TEST_EXES); do \
		echo "Running $$test..."; \
		if ./$$test; then \
			echo "✓ $$test passed"; \
		else \
			echo "✗ $$test failed"; \
			failed=$$((failed + 1)); \
		fi; \
		echo ""; \
	done; \
	if [ $$failed -eq 0 ]; then \
		echo "All tests passed!"; \
	else \
		echo "$$failed test(s) failed"; \
		exit 1; \
	fi

# Run specific test
test-%: $(BUILD_DIR)/test_%
	./$(BUILD_DIR)/test_$*

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f *.gcda *.gcno *.gcov

# Help
help:
	@echo "Available targets:"
	@echo "  all     - Build all tests"
	@echo "  test    - Run all tests"
	@echo "  test-X  - Run specific test (e.g., test-mode_manager)"
	@echo "  clean   - Remove build artifacts"
	@echo "  help    - Show this help message"