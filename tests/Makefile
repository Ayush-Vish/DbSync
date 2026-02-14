# Makefile for DbSync Tests

CXX := g++
CXXFLAGS := -std=c++20 -O2 -pthread -Wall -Wextra -Wno-unused-parameter
INCLUDES := -I../include
LIBS := -luring -labsl_base -labsl_hash -labsl_raw_hash_set

# Test executables
TESTS := test_itc_queue test_main_integration

# Default target
all: $(TESTS)
	@echo "All tests compiled successfully!"

# Compile test_itc_queue
test_itc_queue: test_itc_queue.cpp
	@echo "Compiling test_itc_queue..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LIBS)
	@echo "✓ test_itc_queue compiled"

# Compile test_main_integration
test_main_integration: test_main_integration.cpp
	@echo "Compiling test_main_integration..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LIBS)
	@echo "✓ test_main_integration compiled"

# Run all tests
test: all
	@echo ""
	@echo "=========================================="
	@echo "Running Tests"
	@echo "=========================================="
	@echo ""
	@./test_itc_queue
	@echo ""
	@./test_main_integration
	@echo ""
	@echo "=========================================="
	@echo "All tests completed!"
	@echo "=========================================="

# Run individual tests
test_itc: test_itc_queue
	@./test_itc_queue

test_integration: test_main_integration
	@./test_main_integration

# Clean build artifacts
clean:
	@echo "Cleaning test executables..."
	@rm -f $(TESTS)
	@rm -f *.o
	@rm -f appendonly_*.aof
	@echo "Clean complete!"

# Clean and rebuild
rebuild: clean all

# Help target
help:
	@echo "DbSync Test Suite Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all             - Compile all tests (default)"
	@echo "  test            - Compile and run all tests"
	@echo "  test_itc        - Run only ITC queue tests"
	@echo "  test_integration - Run only integration tests"
	@echo "  clean           - Remove compiled test executables"
	@echo "  rebuild         - Clean and rebuild all tests"
	@echo "  help            - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make            # Compile all tests"
	@echo "  make test       # Compile and run all tests"
	@echo "  make test_itc   # Run ITC tests only"
	@echo "  make clean      # Remove executables"

.PHONY: all test test_itc test_integration clean rebuild help