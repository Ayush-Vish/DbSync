# DbSync Test Suite

This directory contains comprehensive unit and integration tests for the DbSync project.

## Test Files

### 1. `test_itc_queue.cpp` (31 tests)
Tests for the Inter-Thread Communication (ITC) queue infrastructure in `include/itc_queue.hpp`.

**ITCMessage Tests (16 tests):**
- Basic message creation and field assignment
- Key/value get/set operations with normal, empty, max size, and overflow cases
- Special character handling
- Null termination verification
- Message type enum handling
- TTL and found flag operations
- Key/value independence
- Memory alignment verification (64-byte alignment)
- Generation tracking

**ITCContext Tests (15 tests):**
- Context initialization with single and multiple reactors
- Push/pop single messages
- FIFO ordering verification
- Bulk dequeue operations (full, partial, empty)
- Event fd signaling and clearing
- Cross-reactor communication
- Response message flows
- High-volume message handling (1000+ messages)
- Multi-sender, single-receiver patterns
- Generation and pipeline index preservation

### 2. `test_main_integration.cpp` (39 tests)
Integration tests for main application components.

**Connection Tests (3 tests):**
- Connection reset functionality
- Generation tracking across resets
- Multiple reset cycles

**RESP Parser Tests (9 tests):**
- GET command parsing
- SET command parsing (with and without TTL)
- PING command parsing
- CONFIG command parsing
- Incomplete command handling
- Empty buffer handling
- Invalid format detection
- Multiple command parsing in pipeline

**DbSyncEngine Tests (10 tests):**
- Basic set/get operations
- Get nonexistent keys
- Set overwrite behavior
- Multiple key operations
- Delete operations
- Sharding consistency
- Multi-reactor ownership
- Key ownership verification
- Fast owner consistency

**ITC Integration Tests (4 tests):**
- GET request/response flow between reactors
- SET request/response flow between reactors
- Stale response detection using generation numbers
- Pipeline index preservation

**RESP Formatting Tests (4 tests):**
- Bulk string formatting
- Nil response formatting
- OK response formatting
- Error response formatting

## Building and Running Tests

### Prerequisites

- **C++20 compatible compiler** (g++ 10+ or clang++ 11+)
- **liburing** - Linux io_uring library
- **Abseil** - Google's C++ library (for flat_hash_map)
- **pthread** - POSIX threads library

### Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential liburing-dev libabsl-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install -y gcc-c++ liburing-devel abseil-cpp-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel liburing abseil-cpp
```

### Compile and Run

#### Option 1: Use the provided script
```bash
cd tests
chmod +x compile_and_run.sh
./compile_and_run.sh
```

#### Option 2: Manual compilation
```bash
cd tests

# Compile test_itc_queue
g++ -std=c++20 -O2 -pthread -Wall -Wextra -I../include \
    test_itc_queue.cpp -o test_itc_queue -luring \
    -labsl_base -labsl_hash -labsl_raw_hash_set

# Compile test_main_integration
g++ -std=c++20 -O2 -pthread -Wall -Wextra -I../include \
    test_main_integration.cpp -o test_main_integration -luring \
    -labsl_base -labsl_hash -labsl_raw_hash_set

# Run tests
./test_itc_queue
./test_main_integration
```

### Expected Output

When all tests pass, you should see:
```
========================================
Test Results:
  Passed: 31
  Failed: 0
========================================
```

## Test Coverage

### What's Covered

1. **ITCMessage** (include/itc_queue.hpp:20-58)
   - All public methods: `set_key()`, `get_key()`, `set_value()`, `get_value()`
   - Boundary conditions: empty strings, max size, overflow
   - Memory safety: null termination, alignment
   - All message types: GET_REQ, SET_REQ, DEL_REQ, RESP

2. **ITCContext** (include/itc_queue.hpp:61-119)
   - All public methods: `init()`, `push()`, `pop()`, `pop_bulk()`, `signal()`, `clear_signal()`, `get_event_fd()`
   - Multi-reactor scenarios
   - MPSC (Multiple Producer, Single Consumer) patterns
   - High-volume throughput

3. **Connection** (src/main.cpp:37-68)
   - `reset()` method
   - Generation tracking for stale response detection

4. **RespParser** (include/resp_parser.hpp)
   - All command types: GET, SET, PING, CONFIG
   - Pipeline parsing
   - Error handling

5. **DbSyncEngine** (include/dbsync_engine.hpp)
   - Core operations: `set()`, `get()`, `del()`
   - Sharding logic: `fast_owner()`, `owns_key()`
   - Multi-reactor partitioning

### Edge Cases and Regression Tests

- **String overflow handling**: Keys and values exceeding MAX_KEY_SIZE/MAX_VALUE_SIZE
- **Empty data**: Empty keys, values, and buffers
- **Stale response detection**: Connection generation mismatch
- **Pipeline ordering**: Multiple commands in flight with correct index tracking
- **Cross-reactor communication**: Message passing between different reactor threads
- **Boundary values**: Maximum sizes, zero values, negative cases

## Extending the Tests

To add new tests:

1. Follow the existing test pattern:
```cpp
TEST(test_new_feature) {
    // Setup
    // Execute
    // Assert
}
```

2. Add the test to the main() function:
```cpp
RUN_TEST(test_new_feature);
```

3. Recompile and run.

## Troubleshooting

### Compilation Errors

**Error: "liburing.h: No such file or directory"**
- Install liburing-dev: `sudo apt-get install liburing-dev`

**Error: "absl/container/flat_hash_map.h: No such file or directory"**
- Install Abseil: `sudo apt-get install libabsl-dev`

**Error: undefined reference to pthread functions**
- Add `-pthread` flag to compilation command

### Test Failures

If tests fail:
1. Check that the main application compiles correctly
2. Verify all dependencies are installed
3. Review the specific test failure output for details
4. Ensure you're using C++20 or later

## Performance Notes

These are unit/integration tests focused on correctness, not performance. For performance benchmarking, see:
- `../benchmarks/` directory
- `redis-benchmark` integration tests
- Performance metrics in README.md

## License

Same as the parent DbSync project.