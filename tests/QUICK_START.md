# Quick Start Guide

## 🚀 Run All Tests (One Command)

```bash
cd tests
make test
```

## 📦 Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install -y build-essential liburing-dev libabsl-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install -y gcc-c++ liburing-devel abseil-cpp-devel
```

## 🔨 Build Options

### Option 1: Using Make (Recommended)
```bash
make              # Compile all tests
make test         # Compile and run all tests
make test_itc     # Run only ITC tests
make clean        # Remove executables
```

### Option 2: Using the Shell Script
```bash
./compile_and_run.sh
```

### Option 3: Manual Compilation
```bash
# ITC Queue Tests
g++ -std=c++20 -O2 -pthread -I../include \
    test_itc_queue.cpp -o test_itc_queue \
    -luring -labsl_base -labsl_hash -labsl_raw_hash_set

# Integration Tests
g++ -std=c++20 -O2 -pthread -I../include \
    test_main_integration.cpp -o test_main_integration \
    -luring -labsl_base -labsl_hash -labsl_raw_hash_set

# Run
./test_itc_queue
./test_main_integration
```

## ✅ Expected Results

**test_itc_queue:**
- 31 tests covering ITCMessage and ITCContext
- All tests should PASS

**test_main_integration:**
- 39 tests covering Connection, RESP Parser, DbSyncEngine, and ITC flows
- All tests should PASS

## 📊 What's Being Tested

| Component | File | Tests |
|-----------|------|-------|
| ITCMessage | include/itc_queue.hpp | 16 |
| ITCContext | include/itc_queue.hpp | 15 |
| Connection | src/main.cpp | 3 |
| RespParser | include/resp_parser.hpp | 9 |
| DbSyncEngine | include/dbsync_engine.hpp | 10 |
| ITC Integration | Multiple files | 4 |
| RESP Formatting | Multiple files | 4 |
| **Total** | | **70** |

## 🐛 Troubleshooting

**Problem:** `fatal error: liburing.h: No such file or directory`
```bash
sudo apt-get install liburing-dev
```

**Problem:** `fatal error: absl/container/flat_hash_map.h: No such file or directory`
```bash
sudo apt-get install libabsl-dev
```

**Problem:** `undefined reference to pthread_create`
- Add `-pthread` flag to compilation

**Problem:** Tests compile but fail
- Check that the main application compiles successfully
- Verify you're using C++20 or later
- Review specific test failure messages

## 📖 Documentation

- **README.md** - Full documentation
- **TEST_COVERAGE.md** - Detailed coverage report
- **Makefile** - Build configuration

## 🎯 Key Test Highlights

### Edge Cases Covered
- ✓ String overflow (keys/values exceeding max size)
- ✓ Empty data (empty keys, values, buffers)
- ✓ High volume (1000+ messages in single test)
- ✓ Stale response detection
- ✓ Pipeline ordering
- ✓ Special characters

### Regression Tests
- ✓ Generation counter tracking
- ✓ Connection reset behavior
- ✓ Cross-reactor communication
- ✓ FIFO message ordering

## 💡 Adding New Tests

1. Open `test_itc_queue.cpp` or `test_main_integration.cpp`

2. Add your test function:
```cpp
TEST(test_my_new_feature) {
    // Setup
    // Execute
    // Assert
    ASSERT_EQ(expected, actual);
}
```

3. Register it in main():
```cpp
RUN_TEST(test_my_new_feature);
```

4. Recompile and run:
```bash
make test
```

## 🔍 Test Output

Successful test output:
```
Running test_itc_message_basic_creation...
  PASSED
Running test_itc_message_set_get_key_normal...
  PASSED
...
========================================
Test Results:
  Passed: 70
  Failed: 0
========================================
```

Failed test output:
```
Running test_example...
FAIL: test_example:123 - Expected 5 == 10
  FAILED
...
Test Results:
  Passed: 69
  Failed: 1
========================================
```

## 🏆 Success Criteria

All tests PASS = Ready to deploy! ✓

## 📞 Need Help?

- Check **README.md** for detailed information
- Review **TEST_COVERAGE.md** for coverage details
- Look at existing tests as examples
- File issues in the project repository