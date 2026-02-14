#!/usr/bin/env bash
# Script to compile and run all tests

set -e

echo "=========================================="
echo "Compiling DbSync Tests"
echo "=========================================="

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Detect compiler
CXX=${CXX:-g++}
if ! command -v "$CXX" &> /dev/null; then
    echo -e "${RED}Error: C++ compiler not found. Please install g++ or clang++${NC}"
    exit 1
fi

echo "Using compiler: $CXX"

# Compilation flags
CXXFLAGS="-std=c++20 -O2 -pthread -Wall -Wextra"
INCLUDES="-I../include"
LIBS="-luring"

# Check for abseil library (used by dbsync_engine)
if pkg-config --exists absl_base 2>/dev/null; then
    ABSL_FLAGS=$(pkg-config --cflags --libs absl_base absl_hash absl_flat_hash_map)
    LIBS="$LIBS $ABSL_FLAGS"
else
    echo "Warning: Abseil library not found via pkg-config. Assuming headers are in system path."
    LIBS="$LIBS -labsl_base -labsl_hash -labsl_raw_hash_set"
fi

# Compile test_itc_queue
echo ""
echo "Compiling test_itc_queue..."
$CXX $CXXFLAGS $INCLUDES test_itc_queue.cpp -o test_itc_queue $LIBS
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ test_itc_queue compiled successfully${NC}"
else
    echo -e "${RED}✗ test_itc_queue compilation failed${NC}"
    exit 1
fi

# Compile test_main_integration
echo ""
echo "Compiling test_main_integration..."
$CXX $CXXFLAGS $INCLUDES test_main_integration.cpp -o test_main_integration $LIBS
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ test_main_integration compiled successfully${NC}"
else
    echo -e "${RED}✗ test_main_integration compilation failed${NC}"
    exit 1
fi

echo ""
echo "=========================================="
echo "Running Tests"
echo "=========================================="

# Run test_itc_queue
echo ""
echo "Running test_itc_queue..."
./test_itc_queue
TEST1_RESULT=$?

echo ""
echo "Running test_main_integration..."
./test_main_integration
TEST2_RESULT=$?

# Summary
echo ""
echo "=========================================="
echo "Test Summary"
echo "=========================================="
if [ $TEST1_RESULT -eq 0 ]; then
    echo -e "${GREEN}✓ test_itc_queue: PASSED${NC}"
else
    echo -e "${RED}✗ test_itc_queue: FAILED${NC}"
fi

if [ $TEST2_RESULT -eq 0 ]; then
    echo -e "${GREEN}✓ test_main_integration: PASSED${NC}"
else
    echo -e "${RED}✗ test_main_integration: FAILED${NC}"
fi

echo ""
if [ $TEST1_RESULT -eq 0 ] && [ $TEST2_RESULT -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi