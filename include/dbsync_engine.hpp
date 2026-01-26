
#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <optional>
#include <functional>
#include <chrono>
#include <mutex>
#include <fcntl.h>
#include "absl/container/flat_hash_map.h"
// well this is the class in which is the main Engine of the DBSync server

struct ValueEntry{
    std::string data;
    uint64_t expires_at; // timestamp in milliseconds
};


class DbSyncEngine {
private:
    struct Shard{
        // using swiss table to 
        // std::unordered_map<std::string,std::string> data;
        absl::flat_hash_map<std::string, ValueEntry> data;
    };
    std::vector<Shard> shards; // we split the data into multiple Shards.
    int shard_count;

    int get_shard_index (const  std::string_view&key) {
        return std::hash<std::string_view>{}(key) % shard_count; // -> returns the index of the shard for a given key
        // What if it gives the same index for every key? -> then all keys go to the same shard
    }

    public:
    int aof_fd;
    DbSyncEngine(int num_shards) : shard_count(num_shards), shards(num_shards) {
        aof_fd = open("appendonly.aof", O_WRONLY | O_CREAT | O_APPEND, 0644); // open the aof file in append mode , create if not exists,
        if (aof_fd < 0) perror("Failed to open AOF");
    }
    
    uint64_t get_now(){
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    // The Set Operation
    int get_shard_count() const {
        return shard_count;
    }

    Shard& get_shard(int index) {
        return shards[index];
    }
    void set(const std::string_view key, const std::string_view value, uint64_t ttl_ms = 0) {
        auto &s = shards[get_shard_index(key)];
        uint64_t expiry = ttl_ms > 0 ? get_now() + ttl_ms : 0;

        s.data[std::string(key)] = {std::string(value),expiry};
    }
    // The Get Operation
    std::optional<std::string> get (const std::string_view key   ) {
        auto &s = shards[get_shard_index(key)];
        auto it = s.data.find(std::string(key));
        if(it == s.data.end()) return std::nullopt;
        if(it->second.expires_at != 0&&it->second.expires_at <get_now()){
            s.data.erase(std::string(key));
            return std::nullopt;
        }
        return it->second.data;
    }

    bool del(std::string_view key) {
        auto& s = shards[get_shard_index(key)];
        return s.data.erase(std::string(key)) > 0;
    }
};
