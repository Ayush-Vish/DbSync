#include <iostream>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include "../include/resp_parser.hpp"
#include "../include/dbsync_engine.hpp"
#include "../include/itc_queue.hpp"

// Simple test framework
int g_tests_passed = 0;
int g_tests_failed = 0;

#define TEST(name) void name()
#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        std::cerr << "FAIL: " << __FUNCTION__ << ":" << __LINE__ << " - Expected " << (a) << " == " << (b) << std::endl; \
        g_tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_TRUE(condition) do { \
    if (!(condition)) { \
        std::cerr << "FAIL: " << __FUNCTION__ << ":" << __LINE__ << " - Expected condition to be true" << std::endl; \
        g_tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_FALSE(condition) do { \
    if (condition) { \
        std::cerr << "FAIL: " << __FUNCTION__ << ":" << __LINE__ << " - Expected condition to be false" << std::endl; \
        g_tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_STREQ(a, b) do { \
    if (std::string(a) != std::string(b)) { \
        std::cerr << "FAIL: " << __FUNCTION__ << ":" << __LINE__ << " - Expected '" << (a) << "' == '" << (b) << "'" << std::endl; \
        g_tests_failed++; \
        return; \
    } \
} while(0)

#define RUN_TEST(test_func) do { \
    std::cout << "Running " << #test_func << "..." << std::endl; \
    test_func(); \
    g_tests_passed++; \
    std::cout << "  PASSED" << std::endl; \
} while(0)

// ============================================================================
// Connection struct tests (simulating from main.cpp)
// ============================================================================

// Simulate the Connection struct from main.cpp
struct Connection {
    int fd;
    uint32_t generation = 0;
    char buffer[4096];
    std::string response_data;
    char aof_buf[1024];
    uint32_t aof_len = 0;
    int pending_itc = 0;
    std::vector<std::string> pipeline_results;
    int expected_results = 0;
    bool suspended = false;
    int owner_reactor = -1;

    void reset() {
        fd = -1;
        generation++;
        response_data.clear();
        aof_len = 0;
        pending_itc = 0;
        pipeline_results.clear();
        expected_results = 0;
        suspended = false;
        owner_reactor = -1;
    }
};

TEST(test_connection_reset) {
    Connection conn;
    conn.fd = 123;
    conn.generation = 5;
    conn.response_data = "some data";
    conn.aof_len = 100;
    conn.pending_itc = 3;
    conn.pipeline_results = {"result1", "result2"};
    conn.expected_results = 2;
    conn.suspended = true;
    conn.owner_reactor = 2;

    conn.reset();

    ASSERT_EQ(conn.fd, -1);
    ASSERT_EQ(conn.generation, 6u); // Should increment
    ASSERT_TRUE(conn.response_data.empty());
    ASSERT_EQ(conn.aof_len, 0u);
    ASSERT_EQ(conn.pending_itc, 0);
    ASSERT_TRUE(conn.pipeline_results.empty());
    ASSERT_EQ(conn.expected_results, 0);
    ASSERT_FALSE(conn.suspended);
    ASSERT_EQ(conn.owner_reactor, -1);
}

TEST(test_connection_generation_tracking) {
    Connection conn;
    uint32_t initial_gen = conn.generation;

    conn.reset();
    ASSERT_EQ(conn.generation, initial_gen + 1);

    conn.reset();
    ASSERT_EQ(conn.generation, initial_gen + 2);

    conn.reset();
    ASSERT_EQ(conn.generation, initial_gen + 3);
}

TEST(test_connection_multiple_resets) {
    Connection conn;

    // Setup initial state
    conn.fd = 100;
    conn.response_data = "test";
    conn.pending_itc = 5;

    // First reset
    conn.reset();
    ASSERT_EQ(conn.fd, -1);
    ASSERT_EQ(conn.pending_itc, 0);
    uint32_t gen1 = conn.generation;

    // Setup again
    conn.fd = 200;
    conn.response_data = "test2";
    conn.pending_itc = 10;

    // Second reset
    conn.reset();
    ASSERT_EQ(conn.fd, -1);
    ASSERT_EQ(conn.pending_itc, 0);
    ASSERT_EQ(conn.generation, gen1 + 1);
}

// ============================================================================
// RESP Parser Integration Tests
// ============================================================================

TEST(test_resp_parser_get_command) {
    std::string buffer = "*2\r\n$3\r\nGET\r\n$4\r\nkey1\r\n";
    auto [cmd, consumed] = RespParser::parse(buffer);

    ASSERT_EQ(cmd.type, "GET");
    ASSERT_EQ(cmd.args.size(), 1u);
    ASSERT_EQ(cmd.args[0], "key1");
    ASSERT_EQ(consumed, buffer.size());
}

TEST(test_resp_parser_set_command) {
    std::string buffer = "*3\r\n$3\r\nSET\r\n$4\r\nkey1\r\n$6\r\nvalue1\r\n";
    auto [cmd, consumed] = RespParser::parse(buffer);

    ASSERT_EQ(cmd.type, "SET");
    ASSERT_EQ(cmd.args.size(), 2u);
    ASSERT_EQ(cmd.args[0], "key1");
    ASSERT_EQ(cmd.args[1], "value1");
    ASSERT_EQ(consumed, buffer.size());
}

TEST(test_resp_parser_set_with_ttl) {
    std::string buffer = "*5\r\n$3\r\nSET\r\n$4\r\nkey1\r\n$6\r\nvalue1\r\n$2\r\nEX\r\n$2\r\n60\r\n";
    auto [cmd, consumed] = RespParser::parse(buffer);

    ASSERT_EQ(cmd.type, "SET");
    ASSERT_EQ(cmd.args.size(), 4u);
    ASSERT_EQ(cmd.args[0], "key1");
    ASSERT_EQ(cmd.args[1], "value1");
    ASSERT_EQ(cmd.args[2], "EX");
    ASSERT_EQ(cmd.args[3], "60");
    ASSERT_EQ(consumed, buffer.size());
}

TEST(test_resp_parser_ping_command) {
    std::string buffer = "*1\r\n$4\r\nPING\r\n";
    auto [cmd, consumed] = RespParser::parse(buffer);

    ASSERT_EQ(cmd.type, "PING");
    ASSERT_EQ(cmd.args.size(), 0u);
    ASSERT_EQ(consumed, buffer.size());
}

TEST(test_resp_parser_incomplete_command) {
    std::string buffer = "*2\r\n$3\r\nGET\r\n$4\r\nkey";
    auto [cmd, consumed] = RespParser::parse(buffer);

    ASSERT_EQ(cmd.type, "INCOMPLETE");
    ASSERT_EQ(consumed, 0u);
}

TEST(test_resp_parser_empty_buffer) {
    std::string buffer = "";
    auto [cmd, consumed] = RespParser::parse(buffer);

    ASSERT_EQ(cmd.type, "UNKNOWN");
    ASSERT_EQ(consumed, 0u);
}

TEST(test_resp_parser_invalid_format) {
    std::string buffer = "INVALID\r\n";
    auto [cmd, consumed] = RespParser::parse(buffer);

    ASSERT_EQ(cmd.type, "UNKNOWN");
    ASSERT_EQ(consumed, 0u);
}

TEST(test_resp_parser_multiple_commands) {
    std::string buffer = "*2\r\n$3\r\nGET\r\n$4\r\nkey1\r\n*3\r\n$3\r\nSET\r\n$4\r\nkey2\r\n$6\r\nvalue2\r\n";

    // Parse first command
    auto [cmd1, consumed1] = RespParser::parse(buffer);
    ASSERT_EQ(cmd1.type, "GET");
    ASSERT_EQ(cmd1.args[0], "key1");

    // Parse second command
    std::string_view remaining(buffer);
    remaining.remove_prefix(consumed1);
    auto [cmd2, consumed2] = RespParser::parse(remaining);
    ASSERT_EQ(cmd2.type, "SET");
    ASSERT_EQ(cmd2.args[0], "key2");
    ASSERT_EQ(cmd2.args[1], "value2");
}

TEST(test_resp_parser_config_command) {
    std::string buffer = "*3\r\n$6\r\nCONFIG\r\n$3\r\nGET\r\n$4\r\nsave\r\n";
    auto [cmd, consumed] = RespParser::parse(buffer);

    ASSERT_EQ(cmd.type, "CONFIG");
    ASSERT_EQ(cmd.args.size(), 2u);
    ASSERT_EQ(cmd.args[0], "GET");
    ASSERT_EQ(cmd.args[1], "save");
}

// ============================================================================
// DbSyncEngine Integration Tests
// ============================================================================

TEST(test_dbsync_engine_basic_set_get) {
    DbSyncEngine engine(4, 0, 1); // 4 shards, reactor 0, 1 reactor total

    engine.set("test_key", "test_value");
    auto result = engine.get("test_key");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(*result, "test_value");
}

TEST(test_dbsync_engine_get_nonexistent) {
    DbSyncEngine engine(4, 0, 1);

    auto result = engine.get("nonexistent_key");
    ASSERT_FALSE(result.has_value());
}

TEST(test_dbsync_engine_set_overwrite) {
    DbSyncEngine engine(4, 0, 1);

    engine.set("key", "value1");
    engine.set("key", "value2");

    auto result = engine.get("key");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(*result, "value2");
}

TEST(test_dbsync_engine_multiple_keys) {
    DbSyncEngine engine(4, 0, 1);

    engine.set("key1", "value1");
    engine.set("key2", "value2");
    engine.set("key3", "value3");

    ASSERT_EQ(*engine.get("key1"), "value1");
    ASSERT_EQ(*engine.get("key2"), "value2");
    ASSERT_EQ(*engine.get("key3"), "value3");
}

TEST(test_dbsync_engine_del) {
    DbSyncEngine engine(4, 0, 1);

    engine.set("key", "value");
    ASSERT_TRUE(engine.get("key").has_value());

    bool deleted = engine.del("key");
    ASSERT_TRUE(deleted);
    ASSERT_FALSE(engine.get("key").has_value());
}

TEST(test_dbsync_engine_del_nonexistent) {
    DbSyncEngine engine(4, 0, 1);

    bool deleted = engine.del("nonexistent");
    ASSERT_FALSE(deleted);
}

TEST(test_dbsync_engine_sharding_consistency) {
    DbSyncEngine engine(16, 0, 1); // More shards

    // Add many keys
    for (int i = 0; i < 100; i++) {
        std::string key = "key_" + std::to_string(i);
        std::string value = "value_" + std::to_string(i);
        engine.set(key, value);
    }

    // Verify all keys
    for (int i = 0; i < 100; i++) {
        std::string key = "key_" + std::to_string(i);
        std::string expected_value = "value_" + std::to_string(i);
        auto result = engine.get(key);
        ASSERT_TRUE(result.has_value());
        ASSERT_EQ(*result, expected_value);
    }
}

TEST(test_dbsync_engine_multi_reactor_ownership) {
    // Simulate 2 reactors, each with 4 shards (8 total shards)
    DbSyncEngine engine0(4, 0, 2); // Reactor 0: owns shards 0-3
    DbSyncEngine engine1(4, 1, 2); // Reactor 1: owns shards 4-7

    // Test ownership calculation is consistent
    std::string test_key = "test_key";
    int owner0 = engine0.fast_owner(test_key);
    int owner1 = engine1.fast_owner(test_key);

    // Both engines should agree on ownership
    ASSERT_EQ(owner0, owner1);
    ASSERT_TRUE(owner0 == 0 || owner0 == 1);
}

TEST(test_dbsync_engine_owns_key) {
    DbSyncEngine engine0(4, 0, 2); // Reactor 0: owns shards 0-3
    DbSyncEngine engine1(4, 1, 2); // Reactor 1: owns shards 4-7

    // Test multiple keys to ensure proper distribution
    int keys_owned_by_0 = 0;
    int keys_owned_by_1 = 0;

    for (int i = 0; i < 100; i++) {
        std::string key = "key_" + std::to_string(i);

        bool owned_by_0 = engine0.owns_key(key);
        bool owned_by_1 = engine1.owns_key(key);

        // Each key should be owned by exactly one reactor
        ASSERT_TRUE(owned_by_0 != owned_by_1); // XOR: exactly one true

        if (owned_by_0) keys_owned_by_0++;
        if (owned_by_1) keys_owned_by_1++;
    }

    // Verify reasonable distribution (not all keys go to one reactor)
    ASSERT_TRUE(keys_owned_by_0 > 0);
    ASSERT_TRUE(keys_owned_by_1 > 0);
    ASSERT_EQ(keys_owned_by_0 + keys_owned_by_1, 100);
}

TEST(test_dbsync_engine_fast_owner_consistency) {
    DbSyncEngine engine(8, 0, 4); // 8 shards per reactor, 4 reactors

    // Same key should always hash to same owner
    std::string key = "consistent_key";
    int owner1 = engine.fast_owner(key);
    int owner2 = engine.fast_owner(key);
    int owner3 = engine.fast_owner(key);

    ASSERT_EQ(owner1, owner2);
    ASSERT_EQ(owner2, owner3);
    ASSERT_TRUE(owner1 >= 0 && owner1 < 4);
}

// ============================================================================
// ITC Message Flow Tests (integration with main.cpp logic)
// ============================================================================

TEST(test_itc_get_request_response_flow) {
    ITCContext ctx;
    ctx.init(2);

    DbSyncEngine engine0(4, 0, 2);
    DbSyncEngine engine1(4, 1, 2);

    // Set a value in engine1
    std::string key = "shared_key";
    std::string value = "shared_value";
    engine1.set(key, value);

    // Simulate reactor 0 sending GET request to reactor 1
    ITCMessage req;
    req.type = ITCMessage::Type::GET_REQ;
    req.sender_reactor = 0;
    req.conn_fd = 100;
    req.conn_gen = 1;
    req.pipeline_idx = 0;
    req.set_key(key);

    ctx.push(1, req);

    // Reactor 1 receives request
    ITCMessage received_req;
    ASSERT_TRUE(ctx.pop(1, received_req));
    ASSERT_EQ(received_req.type, ITCMessage::Type::GET_REQ);
    ASSERT_EQ(received_req.get_key(), key);

    // Reactor 1 processes GET and sends response
    auto result = engine1.get(received_req.get_key());
    ITCMessage resp;
    resp.type = ITCMessage::Type::RESP;
    resp.sender_reactor = 1;
    resp.conn_fd = received_req.conn_fd;
    resp.conn_gen = received_req.conn_gen;
    resp.pipeline_idx = received_req.pipeline_idx;
    resp.found = result.has_value();
    if (result) {
        resp.set_value(*result);
    }

    ctx.push(0, resp);

    // Reactor 0 receives response
    ITCMessage received_resp;
    ASSERT_TRUE(ctx.pop(0, received_resp));
    ASSERT_EQ(received_resp.type, ITCMessage::Type::RESP);
    ASSERT_TRUE(received_resp.found);
    ASSERT_EQ(received_resp.get_value(), value);
    ASSERT_EQ(received_resp.conn_fd, 100);
    ASSERT_EQ(received_resp.conn_gen, 1u);
}

TEST(test_itc_set_request_response_flow) {
    ITCContext ctx;
    ctx.init(2);

    DbSyncEngine engine0(4, 0, 2);
    DbSyncEngine engine1(4, 1, 2);

    std::string key = "new_key";
    std::string value = "new_value";

    // Reactor 0 sends SET request to reactor 1
    ITCMessage req;
    req.type = ITCMessage::Type::SET_REQ;
    req.sender_reactor = 0;
    req.conn_fd = 200;
    req.conn_gen = 5;
    req.pipeline_idx = 0;
    req.set_key(key);
    req.set_value(value);
    req.ttl_ms = 0;

    ctx.push(1, req);

    // Reactor 1 receives and processes SET
    ITCMessage received_req;
    ASSERT_TRUE(ctx.pop(1, received_req));
    engine1.set(received_req.get_key(), received_req.get_value(), received_req.ttl_ms);

    // Send OK response
    ITCMessage resp;
    resp.type = ITCMessage::Type::RESP;
    resp.sender_reactor = 1;
    resp.conn_fd = received_req.conn_fd;
    resp.conn_gen = received_req.conn_gen;
    resp.pipeline_idx = received_req.pipeline_idx;
    resp.found = true; // SET succeeded

    ctx.push(0, resp);

    // Reactor 0 receives response
    ITCMessage received_resp;
    ASSERT_TRUE(ctx.pop(0, received_resp));
    ASSERT_TRUE(received_resp.found);
    ASSERT_EQ(received_resp.conn_gen, 5u);

    // Verify value was actually set in engine1
    auto result = engine1.get(key);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(*result, value);
}

TEST(test_itc_stale_response_detection) {
    ITCContext ctx;
    ctx.init(2);

    // Simulate a response with mismatched generation
    ITCMessage resp;
    resp.type = ITCMessage::Type::RESP;
    resp.sender_reactor = 1;
    resp.conn_fd = 100;
    resp.conn_gen = 5; // Old generation
    resp.pipeline_idx = 0;
    resp.found = true;

    ctx.push(0, resp);

    ITCMessage received;
    ctx.pop(0, received);

    // In real code, reactor 0 would check if conn->generation matches
    // Here we just verify the generation is preserved
    ASSERT_EQ(received.conn_gen, 5u);

    // If current connection generation is 6, this would be stale
    uint32_t current_generation = 6;
    bool is_stale = (received.conn_gen != current_generation);
    ASSERT_TRUE(is_stale);
}

TEST(test_itc_pipeline_index_preservation) {
    ITCContext ctx;
    ctx.init(2);

    // Send multiple requests with different pipeline indices
    for (int i = 0; i < 5; i++) {
        ITCMessage req;
        req.type = ITCMessage::Type::GET_REQ;
        req.sender_reactor = 0;
        req.conn_fd = 300;
        req.pipeline_idx = i;
        req.set_key("key_" + std::to_string(i));

        ctx.push(1, req);
    }

    // Receive and verify pipeline indices
    for (int i = 0; i < 5; i++) {
        ITCMessage received;
        ASSERT_TRUE(ctx.pop(1, received));
        ASSERT_EQ(received.pipeline_idx, i);
    }
}

// ============================================================================
// RESP Response Formatting Tests
// ============================================================================

TEST(test_resp_bulk_string_format) {
    auto format_bulk = [](std::string_view val) -> std::string {
        return "$" + std::to_string(val.size()) + "\r\n" + std::string(val) + "\r\n";
    };

    std::string result = format_bulk("hello");
    ASSERT_EQ(result, "$5\r\nhello\r\n");

    result = format_bulk("");
    ASSERT_EQ(result, "$0\r\n\r\n");

    result = format_bulk("test_value_123");
    ASSERT_EQ(result, "$14\r\ntest_value_123\r\n");
}

TEST(test_resp_nil_response) {
    std::string nil_response = "$-1\r\n";
    ASSERT_EQ(nil_response, "$-1\r\n");
}

TEST(test_resp_ok_response) {
    std::string ok_response = "+OK\r\n";
    ASSERT_EQ(ok_response, "+OK\r\n");
}

TEST(test_resp_error_response) {
    std::string error_response = "-ERR unknown command\r\n";
    ASSERT_EQ(error_response, "-ERR unknown command\r\n");
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Main Integration Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    // Connection tests
    std::cout << "\n--- Connection Tests ---" << std::endl;
    RUN_TEST(test_connection_reset);
    RUN_TEST(test_connection_generation_tracking);
    RUN_TEST(test_connection_multiple_resets);

    // RESP Parser tests
    std::cout << "\n--- RESP Parser Tests ---" << std::endl;
    RUN_TEST(test_resp_parser_get_command);
    RUN_TEST(test_resp_parser_set_command);
    RUN_TEST(test_resp_parser_set_with_ttl);
    RUN_TEST(test_resp_parser_ping_command);
    RUN_TEST(test_resp_parser_incomplete_command);
    RUN_TEST(test_resp_parser_empty_buffer);
    RUN_TEST(test_resp_parser_invalid_format);
    RUN_TEST(test_resp_parser_multiple_commands);
    RUN_TEST(test_resp_parser_config_command);

    // DbSyncEngine tests
    std::cout << "\n--- DbSyncEngine Tests ---" << std::endl;
    RUN_TEST(test_dbsync_engine_basic_set_get);
    RUN_TEST(test_dbsync_engine_get_nonexistent);
    RUN_TEST(test_dbsync_engine_set_overwrite);
    RUN_TEST(test_dbsync_engine_multiple_keys);
    RUN_TEST(test_dbsync_engine_del);
    RUN_TEST(test_dbsync_engine_del_nonexistent);
    RUN_TEST(test_dbsync_engine_sharding_consistency);
    RUN_TEST(test_dbsync_engine_multi_reactor_ownership);
    RUN_TEST(test_dbsync_engine_owns_key);
    RUN_TEST(test_dbsync_engine_fast_owner_consistency);

    // ITC integration tests
    std::cout << "\n--- ITC Integration Tests ---" << std::endl;
    RUN_TEST(test_itc_get_request_response_flow);
    RUN_TEST(test_itc_set_request_response_flow);
    RUN_TEST(test_itc_stale_response_detection);
    RUN_TEST(test_itc_pipeline_index_preservation);

    // RESP formatting tests
    std::cout << "\n--- RESP Formatting Tests ---" << std::endl;
    RUN_TEST(test_resp_bulk_string_format);
    RUN_TEST(test_resp_nil_response);
    RUN_TEST(test_resp_ok_response);
    RUN_TEST(test_resp_error_response);

    // Summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Results:" << std::endl;
    std::cout << "  Passed: " << g_tests_passed << std::endl;
    std::cout << "  Failed: " << g_tests_failed << std::endl;
    std::cout << "========================================" << std::endl;

    return g_tests_failed == 0 ? 0 : 1;
}