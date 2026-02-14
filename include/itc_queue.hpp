#pragma once
#include <atomic>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
#include <sys/eventfd.h>
#include <unistd.h>
#include <string_view>
#include <algorithm>

// Avoid conflict with linux kernel BLOCK_SIZE macro
#ifdef BLOCK_SIZE
#undef BLOCK_SIZE
#endif

#include "concurrentqueue.h"

// Fixed-size message to avoid malloc
struct alignas(64) ITCMessage {
    enum class Type : uint8_t {
        GET_REQ,
        SET_REQ,
        DEL_REQ,
        RESP
    };
    
    Type type;
    int sender_reactor;
    int conn_fd;              // Which connection this is for
    int pipeline_idx;         // Position in pipelined response
    uint32_t conn_gen;        // Connection generation to detect stale responses
    
    static constexpr size_t MAX_KEY_SIZE = 128;
    static constexpr size_t MAX_VALUE_SIZE = 384;
    
    char key[MAX_KEY_SIZE];
    char value[MAX_VALUE_SIZE];
    uint16_t key_len = 0;
    uint16_t value_len = 0;
    uint64_t ttl_ms = 0;
    bool found = false;       // For RESP: was key found?
    
    void set_key(std::string_view k) {
        key_len = std::min(k.size(), MAX_KEY_SIZE - 1);
        std::memcpy(key, k.data(), key_len);
        key[key_len] = '\0';
    }
    
    void set_value(std::string_view v) {
        value_len = std::min(v.size(), MAX_VALUE_SIZE - 1);
        std::memcpy(value, v.data(), value_len);
        value[value_len] = '\0';
    }
    
    std::string_view get_key() const { return {key, key_len}; }
    std::string_view get_value() const { return {value, value_len}; }
};

// Global ITC infrastructure using moodycamel ConcurrentQueue
struct ITCContext {
    int num_reactors = 0;
    
    // Each reactor has ONE inbox - all other reactors push to it (MPSC pattern)
    std::vector<std::unique_ptr<moodycamel::ConcurrentQueue<ITCMessage>>> inboxes;
    std::vector<int> event_fds;
    
    void init(int n_reactors) {
        num_reactors = n_reactors;
        
        // One inbox per reactor (receives from all others)
        inboxes.resize(n_reactors);
        for (int i = 0; i < n_reactors; i++) {
            inboxes[i] = std::make_unique<moodycamel::ConcurrentQueue<ITCMessage>>(4096);
        }
        
        event_fds.resize(n_reactors);
        for (int i = 0; i < n_reactors; i++) {
            event_fds[i] = eventfd(0, EFD_NONBLOCK);
        }
    }
    
    // Push message to target reactor's inbox
    bool push(int target_reactor, const ITCMessage& msg) {
        return inboxes[target_reactor]->enqueue(msg);
    }
    
    // Pop message from this reactor's inbox
    bool pop(int reactor_id, ITCMessage& msg) {
        return inboxes[reactor_id]->try_dequeue(msg);
    }
    
    // Bulk dequeue for efficiency
    size_t pop_bulk(int reactor_id, ITCMessage* msgs, size_t max) {
        return inboxes[reactor_id]->try_dequeue_bulk(msgs, max);
    }
    
    int get_event_fd(int reactor_id) const {
        return event_fds[reactor_id];
    }
    
    // Signal a reactor to wake up (returns true if signal was sent)
    bool signal(int reactor_id) {
        uint64_t val = 1;
        return write(event_fds[reactor_id], &val, sizeof(val)) == sizeof(val);
    }
    
    // Clear signal (call after waking)
    void clear_signal(int reactor_id) {
        uint64_t val;
        [[maybe_unused]] auto _ = read(event_fds[reactor_id], &val, sizeof(val));
    }
    
    ~ITCContext() {
        for (int fd : event_fds) {
            if (fd >= 0) close(fd);
        }
    }
};

