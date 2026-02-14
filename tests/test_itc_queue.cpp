#include <iostream>
#include <cassert>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
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
// ITCMessage Tests
// ============================================================================

TEST(test_itc_message_basic_creation) {
    ITCMessage msg;
    msg.type = ITCMessage::Type::GET_REQ;
    msg.sender_reactor = 1;
    msg.conn_fd = 42;
    msg.pipeline_idx = 0;
    msg.conn_gen = 100;

    ASSERT_EQ(msg.type, ITCMessage::Type::GET_REQ);
    ASSERT_EQ(msg.sender_reactor, 1);
    ASSERT_EQ(msg.conn_fd, 42);
    ASSERT_EQ(msg.pipeline_idx, 0);
    ASSERT_EQ(msg.conn_gen, 100u);
}

TEST(test_itc_message_set_get_key_normal) {
    ITCMessage msg;
    std::string test_key = "test_key";
    msg.set_key(test_key);

    ASSERT_EQ(msg.key_len, test_key.size());
    ASSERT_STREQ(msg.key, "test_key");
    ASSERT_EQ(msg.get_key(), test_key);
}

TEST(test_itc_message_set_get_key_empty) {
    ITCMessage msg;
    msg.set_key("");

    ASSERT_EQ(msg.key_len, 0u);
    ASSERT_EQ(msg.get_key().size(), 0u);
    ASSERT_TRUE(msg.get_key().empty());
}

TEST(test_itc_message_set_get_key_max_size) {
    ITCMessage msg;
    std::string long_key(ITCMessage::MAX_KEY_SIZE - 1, 'x');
    msg.set_key(long_key);

    ASSERT_EQ(msg.key_len, ITCMessage::MAX_KEY_SIZE - 1);
    ASSERT_EQ(msg.get_key().size(), ITCMessage::MAX_KEY_SIZE - 1);
    ASSERT_EQ(msg.get_key(), long_key);
}

TEST(test_itc_message_set_get_key_overflow) {
    ITCMessage msg;
    // Try to set a key longer than MAX_KEY_SIZE
    std::string too_long_key(ITCMessage::MAX_KEY_SIZE + 100, 'y');
    msg.set_key(too_long_key);

    // Should be truncated to MAX_KEY_SIZE - 1
    ASSERT_EQ(msg.key_len, ITCMessage::MAX_KEY_SIZE - 1);
    ASSERT_EQ(msg.get_key().size(), ITCMessage::MAX_KEY_SIZE - 1);

    // Verify truncation happened correctly
    std::string expected = too_long_key.substr(0, ITCMessage::MAX_KEY_SIZE - 1);
    ASSERT_EQ(msg.get_key(), expected);
}

TEST(test_itc_message_set_get_value_normal) {
    ITCMessage msg;
    std::string test_value = "test_value_123";
    msg.set_value(test_value);

    ASSERT_EQ(msg.value_len, test_value.size());
    ASSERT_STREQ(msg.value, "test_value_123");
    ASSERT_EQ(msg.get_value(), test_value);
}

TEST(test_itc_message_set_get_value_empty) {
    ITCMessage msg;
    msg.set_value("");

    ASSERT_EQ(msg.value_len, 0u);
    ASSERT_EQ(msg.get_value().size(), 0u);
    ASSERT_TRUE(msg.get_value().empty());
}

TEST(test_itc_message_set_get_value_max_size) {
    ITCMessage msg;
    std::string long_value(ITCMessage::MAX_VALUE_SIZE - 1, 'v');
    msg.set_value(long_value);

    ASSERT_EQ(msg.value_len, ITCMessage::MAX_VALUE_SIZE - 1);
    ASSERT_EQ(msg.get_value().size(), ITCMessage::MAX_VALUE_SIZE - 1);
    ASSERT_EQ(msg.get_value(), long_value);
}

TEST(test_itc_message_set_get_value_overflow) {
    ITCMessage msg;
    // Try to set a value longer than MAX_VALUE_SIZE
    std::string too_long_value(ITCMessage::MAX_VALUE_SIZE + 200, 'z');
    msg.set_value(too_long_value);

    // Should be truncated to MAX_VALUE_SIZE - 1
    ASSERT_EQ(msg.value_len, ITCMessage::MAX_VALUE_SIZE - 1);
    ASSERT_EQ(msg.get_value().size(), ITCMessage::MAX_VALUE_SIZE - 1);

    // Verify truncation happened correctly
    std::string expected = too_long_value.substr(0, ITCMessage::MAX_VALUE_SIZE - 1);
    ASSERT_EQ(msg.get_value(), expected);
}

TEST(test_itc_message_all_types) {
    ITCMessage msg1, msg2, msg3, msg4;

    msg1.type = ITCMessage::Type::GET_REQ;
    msg2.type = ITCMessage::Type::SET_REQ;
    msg3.type = ITCMessage::Type::DEL_REQ;
    msg4.type = ITCMessage::Type::RESP;

    ASSERT_EQ(msg1.type, ITCMessage::Type::GET_REQ);
    ASSERT_EQ(msg2.type, ITCMessage::Type::SET_REQ);
    ASSERT_EQ(msg3.type, ITCMessage::Type::DEL_REQ);
    ASSERT_EQ(msg4.type, ITCMessage::Type::RESP);
}

TEST(test_itc_message_ttl_and_found) {
    ITCMessage msg;
    msg.ttl_ms = 60000; // 60 seconds
    msg.found = true;

    ASSERT_EQ(msg.ttl_ms, 60000u);
    ASSERT_TRUE(msg.found);

    msg.found = false;
    ASSERT_FALSE(msg.found);
}

TEST(test_itc_message_multiple_set_key_calls) {
    ITCMessage msg;

    msg.set_key("first_key");
    ASSERT_EQ(msg.get_key(), "first_key");

    msg.set_key("second_key_longer");
    ASSERT_EQ(msg.get_key(), "second_key_longer");

    msg.set_key("k");
    ASSERT_EQ(msg.get_key(), "k");
}

TEST(test_itc_message_key_value_independence) {
    ITCMessage msg;

    msg.set_key("my_key");
    msg.set_value("my_value");

    ASSERT_EQ(msg.get_key(), "my_key");
    ASSERT_EQ(msg.get_value(), "my_value");

    // Changing key shouldn't affect value
    msg.set_key("new_key");
    ASSERT_EQ(msg.get_key(), "new_key");
    ASSERT_EQ(msg.get_value(), "my_value");

    // Changing value shouldn't affect key
    msg.set_value("new_value");
    ASSERT_EQ(msg.get_key(), "new_key");
    ASSERT_EQ(msg.get_value(), "new_value");
}

TEST(test_itc_message_special_characters) {
    ITCMessage msg;

    // Test with special characters
    std::string special_key = "key:with:colons";
    std::string special_value = "value\nwith\nnewlines\tand\ttabs";

    msg.set_key(special_key);
    msg.set_value(special_value);

    ASSERT_EQ(msg.get_key(), special_key);
    ASSERT_EQ(msg.get_value(), special_value);
}

// ============================================================================
// ITCContext Tests
// ============================================================================

TEST(test_itc_context_init_single_reactor) {
    ITCContext ctx;
    ctx.init(1);

    ASSERT_EQ(ctx.num_reactors, 1);
    ASSERT_EQ(ctx.inboxes.size(), 1u);
    ASSERT_EQ(ctx.event_fds.size(), 1u);
    ASSERT_TRUE(ctx.event_fds[0] >= 0); // Valid fd
}

TEST(test_itc_context_init_multiple_reactors) {
    ITCContext ctx;
    int num_reactors = 4;
    ctx.init(num_reactors);

    ASSERT_EQ(ctx.num_reactors, num_reactors);
    ASSERT_EQ(ctx.inboxes.size(), static_cast<size_t>(num_reactors));
    ASSERT_EQ(ctx.event_fds.size(), static_cast<size_t>(num_reactors));

    // Check all eventfds are valid
    for (int i = 0; i < num_reactors; i++) {
        ASSERT_TRUE(ctx.event_fds[i] >= 0);
    }
}

TEST(test_itc_context_push_pop_single_message) {
    ITCContext ctx;
    ctx.init(2);

    ITCMessage msg;
    msg.type = ITCMessage::Type::GET_REQ;
    msg.sender_reactor = 0;
    msg.conn_fd = 123;
    msg.set_key("test_key");

    // Push from reactor 0 to reactor 1
    bool pushed = ctx.push(1, msg);
    ASSERT_TRUE(pushed);

    // Pop from reactor 1
    ITCMessage received;
    bool popped = ctx.pop(1, received);
    ASSERT_TRUE(popped);

    // Verify message contents
    ASSERT_EQ(received.type, ITCMessage::Type::GET_REQ);
    ASSERT_EQ(received.sender_reactor, 0);
    ASSERT_EQ(received.conn_fd, 123);
    ASSERT_EQ(received.get_key(), "test_key");
}

TEST(test_itc_context_pop_empty_queue) {
    ITCContext ctx;
    ctx.init(1);

    ITCMessage msg;
    bool popped = ctx.pop(0, msg);

    ASSERT_FALSE(popped); // Queue is empty
}

TEST(test_itc_context_multiple_messages_fifo) {
    ITCContext ctx;
    ctx.init(2);

    // Push multiple messages
    for (int i = 0; i < 5; i++) {
        ITCMessage msg;
        msg.type = ITCMessage::Type::GET_REQ;
        msg.sender_reactor = 0;
        msg.conn_fd = i;
        msg.set_key("key_" + std::to_string(i));

        bool pushed = ctx.push(1, msg);
        ASSERT_TRUE(pushed);
    }

    // Pop them in FIFO order
    for (int i = 0; i < 5; i++) {
        ITCMessage received;
        bool popped = ctx.pop(1, received);
        ASSERT_TRUE(popped);
        ASSERT_EQ(received.conn_fd, i);
        ASSERT_EQ(received.get_key(), "key_" + std::to_string(i));
    }

    // Queue should be empty now
    ITCMessage extra;
    ASSERT_FALSE(ctx.pop(1, extra));
}

TEST(test_itc_context_push_pop_bulk) {
    ITCContext ctx;
    ctx.init(2);

    const int num_messages = 10;

    // Push multiple messages
    for (int i = 0; i < num_messages; i++) {
        ITCMessage msg;
        msg.type = ITCMessage::Type::SET_REQ;
        msg.sender_reactor = 0;
        msg.conn_fd = 100 + i;
        msg.set_key("bulk_key_" + std::to_string(i));

        ctx.push(1, msg);
    }

    // Pop bulk
    ITCMessage messages[num_messages];
    size_t dequeued = ctx.pop_bulk(1, messages, num_messages);

    ASSERT_EQ(dequeued, static_cast<size_t>(num_messages));

    // Verify messages
    for (int i = 0; i < num_messages; i++) {
        ASSERT_EQ(messages[i].conn_fd, 100 + i);
        ASSERT_EQ(messages[i].get_key(), "bulk_key_" + std::to_string(i));
    }
}

TEST(test_itc_context_pop_bulk_partial) {
    ITCContext ctx;
    ctx.init(2);

    // Push 5 messages
    for (int i = 0; i < 5; i++) {
        ITCMessage msg;
        msg.type = ITCMessage::Type::GET_REQ;
        msg.conn_fd = i;
        ctx.push(1, msg);
    }

    // Try to pop 10, should get only 5
    ITCMessage messages[10];
    size_t dequeued = ctx.pop_bulk(1, messages, 10);

    ASSERT_EQ(dequeued, 5u);
}

TEST(test_itc_context_pop_bulk_empty) {
    ITCContext ctx;
    ctx.init(2);

    ITCMessage messages[5];
    size_t dequeued = ctx.pop_bulk(1, messages, 5);

    ASSERT_EQ(dequeued, 0u);
}

TEST(test_itc_context_signal_and_clear) {
    ITCContext ctx;
    ctx.init(2);

    // Signal reactor 1
    bool signaled = ctx.signal(1);
    ASSERT_TRUE(signaled);

    // Clear signal
    ctx.clear_signal(1);

    // Signal again to verify it still works
    signaled = ctx.signal(1);
    ASSERT_TRUE(signaled);

    ctx.clear_signal(1);
}

TEST(test_itc_context_get_event_fd) {
    ITCContext ctx;
    ctx.init(3);

    for (int i = 0; i < 3; i++) {
        int fd = ctx.get_event_fd(i);
        ASSERT_TRUE(fd >= 0);
    }
}

TEST(test_itc_context_cross_reactor_communication) {
    ITCContext ctx;
    ctx.init(4);

    // Reactor 0 sends to reactor 2
    ITCMessage msg1;
    msg1.type = ITCMessage::Type::GET_REQ;
    msg1.sender_reactor = 0;
    msg1.set_key("key_from_0");
    ctx.push(2, msg1);

    // Reactor 3 sends to reactor 1
    ITCMessage msg2;
    msg2.type = ITCMessage::Type::SET_REQ;
    msg2.sender_reactor = 3;
    msg2.set_key("key_from_3");
    ctx.push(1, msg2);

    // Reactor 2 receives from reactor 0
    ITCMessage recv1;
    ASSERT_TRUE(ctx.pop(2, recv1));
    ASSERT_EQ(recv1.sender_reactor, 0);
    ASSERT_EQ(recv1.get_key(), "key_from_0");

    // Reactor 1 receives from reactor 3
    ITCMessage recv2;
    ASSERT_TRUE(ctx.pop(1, recv2));
    ASSERT_EQ(recv2.sender_reactor, 3);
    ASSERT_EQ(recv2.get_key(), "key_from_3");
}

TEST(test_itc_context_response_message_flow) {
    ITCContext ctx;
    ctx.init(2);

    // Reactor 0 sends GET_REQ to reactor 1
    ITCMessage req;
    req.type = ITCMessage::Type::GET_REQ;
    req.sender_reactor = 0;
    req.conn_fd = 456;
    req.conn_gen = 10;
    req.pipeline_idx = 0;
    req.set_key("requested_key");
    ctx.push(1, req);

    // Reactor 1 receives and processes
    ITCMessage received_req;
    ASSERT_TRUE(ctx.pop(1, received_req));
    ASSERT_EQ(received_req.type, ITCMessage::Type::GET_REQ);

    // Reactor 1 sends RESP back to reactor 0
    ITCMessage resp;
    resp.type = ITCMessage::Type::RESP;
    resp.sender_reactor = 1;
    resp.conn_fd = received_req.conn_fd;
    resp.conn_gen = received_req.conn_gen;
    resp.pipeline_idx = received_req.pipeline_idx;
    resp.found = true;
    resp.set_value("response_value");
    ctx.push(0, resp);

    // Reactor 0 receives response
    ITCMessage received_resp;
    ASSERT_TRUE(ctx.pop(0, received_resp));
    ASSERT_EQ(received_resp.type, ITCMessage::Type::RESP);
    ASSERT_EQ(received_resp.sender_reactor, 1);
    ASSERT_EQ(received_resp.conn_fd, 456);
    ASSERT_EQ(received_resp.conn_gen, 10u);
    ASSERT_TRUE(received_resp.found);
    ASSERT_EQ(received_resp.get_value(), "response_value");
}

TEST(test_itc_context_high_volume_messages) {
    ITCContext ctx;
    ctx.init(2);

    const int num_messages = 1000;

    // Push many messages
    for (int i = 0; i < num_messages; i++) {
        ITCMessage msg;
        msg.type = ITCMessage::Type::SET_REQ;
        msg.conn_fd = i;
        msg.set_key("key_" + std::to_string(i));
        msg.set_value("value_" + std::to_string(i));

        bool pushed = ctx.push(1, msg);
        ASSERT_TRUE(pushed);
    }

    // Pop all messages and verify
    int received_count = 0;
    while (true) {
        ITCMessage msg;
        if (!ctx.pop(1, msg)) break;

        ASSERT_EQ(msg.conn_fd, received_count);
        ASSERT_EQ(msg.get_key(), "key_" + std::to_string(received_count));
        ASSERT_EQ(msg.get_value(), "value_" + std::to_string(received_count));
        received_count++;
    }

    ASSERT_EQ(received_count, num_messages);
}

TEST(test_itc_message_alignment) {
    // Verify that ITCMessage is properly aligned to 64 bytes
    ASSERT_EQ(alignof(ITCMessage), 64u);
}

TEST(test_itc_context_generation_tracking) {
    ITCContext ctx;
    ctx.init(2);

    // Test that generation numbers are preserved through push/pop
    ITCMessage msg;
    msg.type = ITCMessage::Type::GET_REQ;
    msg.sender_reactor = 0;
    msg.conn_fd = 789;
    msg.conn_gen = 42;
    msg.pipeline_idx = 3;

    ctx.push(1, msg);

    ITCMessage received;
    ctx.pop(1, received);

    ASSERT_EQ(received.conn_gen, 42u);
    ASSERT_EQ(received.pipeline_idx, 3);
}

// Additional edge case test: concurrent access simulation
TEST(test_itc_context_multi_sender_single_receiver) {
    ITCContext ctx;
    ctx.init(5);

    // Multiple reactors (0-3) send to reactor 4
    for (int sender = 0; sender < 4; sender++) {
        ITCMessage msg;
        msg.type = ITCMessage::Type::GET_REQ;
        msg.sender_reactor = sender;
        msg.conn_fd = 1000 + sender;
        msg.set_key("key_from_" + std::to_string(sender));

        ctx.push(4, msg);
    }

    // Reactor 4 receives all messages
    std::vector<int> received_senders;
    for (int i = 0; i < 4; i++) {
        ITCMessage msg;
        bool popped = ctx.pop(4, msg);
        ASSERT_TRUE(popped);
        received_senders.push_back(msg.sender_reactor);
    }

    // Verify all senders were received (order may vary)
    ASSERT_EQ(received_senders.size(), 4u);

    // No more messages
    ITCMessage extra;
    ASSERT_FALSE(ctx.pop(4, extra));
}

TEST(test_itc_message_null_termination) {
    ITCMessage msg;

    std::string test_key = "test";
    std::string test_value = "value";

    msg.set_key(test_key);
    msg.set_value(test_value);

    // Verify null termination
    ASSERT_EQ(msg.key[test_key.size()], '\0');
    ASSERT_EQ(msg.value[test_value.size()], '\0');

    // Verify we can use as C strings
    ASSERT_STREQ(msg.key, "test");
    ASSERT_STREQ(msg.value, "value");
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running ITC Queue Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    // ITCMessage tests
    std::cout << "\n--- ITCMessage Tests ---" << std::endl;
    RUN_TEST(test_itc_message_basic_creation);
    RUN_TEST(test_itc_message_set_get_key_normal);
    RUN_TEST(test_itc_message_set_get_key_empty);
    RUN_TEST(test_itc_message_set_get_key_max_size);
    RUN_TEST(test_itc_message_set_get_key_overflow);
    RUN_TEST(test_itc_message_set_get_value_normal);
    RUN_TEST(test_itc_message_set_get_value_empty);
    RUN_TEST(test_itc_message_set_get_value_max_size);
    RUN_TEST(test_itc_message_set_get_value_overflow);
    RUN_TEST(test_itc_message_all_types);
    RUN_TEST(test_itc_message_ttl_and_found);
    RUN_TEST(test_itc_message_multiple_set_key_calls);
    RUN_TEST(test_itc_message_key_value_independence);
    RUN_TEST(test_itc_message_special_characters);
    RUN_TEST(test_itc_message_alignment);
    RUN_TEST(test_itc_message_null_termination);

    // ITCContext tests
    std::cout << "\n--- ITCContext Tests ---" << std::endl;
    RUN_TEST(test_itc_context_init_single_reactor);
    RUN_TEST(test_itc_context_init_multiple_reactors);
    RUN_TEST(test_itc_context_push_pop_single_message);
    RUN_TEST(test_itc_context_pop_empty_queue);
    RUN_TEST(test_itc_context_multiple_messages_fifo);
    RUN_TEST(test_itc_context_push_pop_bulk);
    RUN_TEST(test_itc_context_pop_bulk_partial);
    RUN_TEST(test_itc_context_pop_bulk_empty);
    RUN_TEST(test_itc_context_signal_and_clear);
    RUN_TEST(test_itc_context_get_event_fd);
    RUN_TEST(test_itc_context_cross_reactor_communication);
    RUN_TEST(test_itc_context_response_message_flow);
    RUN_TEST(test_itc_context_high_volume_messages);
    RUN_TEST(test_itc_context_generation_tracking);
    RUN_TEST(test_itc_context_multi_sender_single_receiver);

    // Summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Results:" << std::endl;
    std::cout << "  Passed: " << g_tests_passed << std::endl;
    std::cout << "  Failed: " << g_tests_failed << std::endl;
    std::cout << "========================================" << std::endl;

    return g_tests_failed == 0 ? 0 : 1;
}