#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <stack>

// Constants
constexpr int PORT = 6379;
constexpr int QUEUE_DEPTH = 4096;
constexpr int MAX_CONN_PER_THREAD = 4096;

enum class OpType
{
    ACCEPT,
    READ,
    WRITE,
    AOF_WRITE,
    ITC_EVENT
};

struct Connection
{
    int fd = -1;
    OpType type = OpType::READ;
    char buffer[4096];
    std::string response_data;

    // ITC tracking for shared-nothing architecture
    int pending_itc = 0;
    std::vector<std::string> pipeline_results;
    int expected_results = 0;
    bool suspended = false;
    int owner_reactor = -1;

    void reset()
    {
        fd = -1;
        response_data.clear();
        pending_itc = 0;
        pipeline_results.clear();
        expected_results = 0;
        suspended = false;
        owner_reactor = -1;
    }
};

// Connection pool helper
class ConnectionPool
{
    std::vector<Connection> pool;
    std::stack<int> free_indices;

public:
    explicit ConnectionPool(int size) : pool(size)
    {
        for (int i = size - 1; i >= 0; --i)
        {
            pool[i].reset();
            free_indices.push(i);
        }
    }

    Connection* acquire()
    {
        if (free_indices.empty())
            return nullptr;
        int idx = free_indices.top();
        free_indices.pop();
        return &pool[idx];
    }

    void release(Connection* conn)
    {
        conn->reset();
        free_indices.push(conn - &pool[0]);
    }
};
