#pragma once

#include <string>
#include <string_view>
#include <charconv>
#include "connection.hpp"
#include "resp_parser.hpp"
#include "dbsync_engine.hpp"
#include "itc_queue.hpp"

// RESP format helpers
namespace resp {

inline std::string bulk(std::string_view val)
{
    return "$" + std::to_string(val.size()) + "\r\n" + std::string(val) + "\r\n";
}

constexpr std::string_view OK = "+OK\r\n";
constexpr std::string_view NIL = "$-1\r\n";
constexpr std::string_view PONG = "+PONG\r\n";
constexpr std::string_view EMPTY_ARRAY = "*0\r\n";
constexpr std::string_view ERR_UNKNOWN = "-ERR unknown command\r\n";

} // namespace resp

// Result of processing a single command
struct CommandResult
{
    std::string response;
    bool needs_itc = false;      // Forwarded to another reactor
    int target_reactor = -1;     // Which reactor to forward to
    bool needs_aof = false;      // Needs AOF write
};

// Parse TTL from SET command arguments
inline uint64_t parse_ttl(const Command& cmd)
{
    uint64_t ttl_ms = 0;
    
    if (cmd.args.size() >= 4)
    {
        std::string_view flag = cmd.args[2];
        std::string_view timeout = cmd.args[3];
        uint64_t val = 0;
        
        auto [ptr, ec] = std::from_chars(timeout.data(), timeout.data() + timeout.size(), val);
        
        if (ec == std::errc())
        {
            if (flag == "EX" || flag == "ex")
                ttl_ms = val * 1000;
            else if (flag == "PX" || flag == "px")
                ttl_ms = val;
        }
    }
    else if (cmd.args.size() == 3)
    {
        std::from_chars(cmd.args[2].data(), cmd.args[2].data() + cmd.args[2].size(), ttl_ms);
    }
    
    return ttl_ms;
}

// Process a command locally (when reactor owns the key)
inline std::string process_local_get(DbSyncEngine& engine, std::string_view key)
{
    auto val = engine.get(key);
    return val ? resp::bulk(*val) : std::string(resp::NIL);
}

inline std::string process_local_set(DbSyncEngine& engine, std::string_view key, 
                                     std::string_view value, uint64_t ttl_ms)
{
    engine.set(key, value, ttl_ms);
    return std::string(resp::OK);
}
