
#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <optional>
#include <functional>
#include <mutex>
#include "absl/container/flat_hash_map.h"
// well this is the class in which is the main Engine of the DBSync server

class DbSyncEngine {
private:
    struct Shard{
        // using swiss table to 
        // std::unordered_map<std::string,std::string> data;
        absl::flat_hash_map<std::string,std::string> data;
        std::shared_mutex mtx; // shared_mutex lets many people read at once, but only one write
    };
    std::vector<Shard> shards; // we split the data into multiple Shards.
    int shard_count;

    int get_shard_index (const  std::string_view&key) {
        return std::hash<std::string_view>{}(key) % shard_count; // -> returns the index of the shard for a given key
        // What if it gives the same index for every key? -> then all keys go to the same shard
    }
public:
    DbSyncEngine(int num_shards) : shard_count(num_shards), shards(num_shards) {}

    // The Set Operation

    void set(const std::string_view key, const std::string_view value) {
        auto &s = shards[get_shard_index(key)];
        std::unique_lock lock(s.mtx);
        s.data[std::string(key)] = std::string(value);
    }
    // The Get Operation
    std::optional<std::string> get (const std::string_view key   ) {
        auto &s = shards[get_shard_index(key)];
        std::shared_lock lock(s.mtx);
        auto it = s.data.find(std::string(key));
        if(it != s.data.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool del(std::string_view key) {
        auto& s = shards[get_shard_index(key)];
        std::unique_lock lock(s.mtx); // we need to lock the shard to delete safely
        return s.data.erase(std::string(key)) > 0;
    }
};
