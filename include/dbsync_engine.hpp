
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
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <charconv>
#include "absl/container/flat_hash_map.h"

struct ValueEntry {
    std::string data;
    uint64_t expires_at; // timestamp in milliseconds
};

class DbSyncEngine {
private:
    struct Shard {
        absl::flat_hash_map<std::string, ValueEntry> data;
    };
    std::vector<Shard> shards;
    int shard_count;        // Local shards per reactor
    int num_reactors;       // MUST be before total_shards!
    int total_shards;       // Global total shards across all reactors
    int reactor_id;
    int shard_start;        // First global shard this reactor owns
    int shard_end;          // Last global shard (exclusive) this reactor owns
    std::string file_name;

    // FNV-1a hash for fast key ownership determination
    static constexpr uint64_t fnv1a_hash(std::string_view key) {
        uint64_t hash = 14695981039346656037ULL;
        for (char c : key) {
            hash ^= static_cast<uint64_t>(c);
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    // Get global shard index for a key
    int get_global_shard_index(std::string_view key) const {
        return fnv1a_hash(key) % total_shards;
    }

    // Get local shard index (within this reactor's shards)
    int get_local_shard_index(std::string_view key) const {
        int global_idx = get_global_shard_index(key);
        return global_idx - shard_start;
    }

public:
    // Determine which reactor owns a key
    int fast_owner(std::string_view key) const {
        int global_shard = get_global_shard_index(key);
        return global_shard / shard_count;  // shard_count = shards per reactor
    }

    // Check if this reactor owns a key
    bool owns_key(std::string_view key) const {
        int global_shard = get_global_shard_index(key);
        return global_shard >= shard_start && global_shard < shard_end;
    }

    int get_reactor_id() const { return reactor_id; }
    int get_num_reactors() const { return num_reactors; }

    // Parse a single RESP bulk string, returns the string and advances pos
    std::string_view parse_bulk_string(const char* data, size_t len, size_t& pos) {
        if (pos >= len || data[pos] != '$') return {};
        
        size_t line_end = pos;
        while (line_end < len && data[line_end] != '\r') line_end++;
        if (line_end + 1 >= len) return {};
        
        int str_len = 0;
        std::from_chars(data + pos + 1, data + line_end, str_len);
        pos = line_end + 2; // skip \r\n
        
        if (pos + str_len > len) return {};
        std::string_view result(data + pos, str_len);
        pos += str_len + 2; // skip string + \r\n
        return result;
    }

public:
    int aof_fd = -1;

    // Constructor with global shard partitioning
    // shards_per_reactor: number of shards this reactor manages locally
    // reactor_id: this reactor's ID (0 to num_reactors-1)
    // num_reactors: total number of reactors
    DbSyncEngine(int shards_per_reactor, int reactor_id = -1, int num_reactors = 1) 
        : shards(shards_per_reactor),
          shard_count(shards_per_reactor), 
          num_reactors(num_reactors),
          total_shards(shards_per_reactor * num_reactors),
          reactor_id(reactor_id) {
        
        // Calculate which global shards this reactor owns
        shard_start = reactor_id * shard_count;
        shard_end = shard_start + shard_count;
        
        if (reactor_id != -1) {
            file_name = "appendonly_" + std::to_string(reactor_id) + ".aof";
            
            // First recover existing data
            recover_from_aof();
            
            // Then open for append
            aof_fd = open(file_name.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_NOATIME, 0644);
            if (aof_fd < 0) {
                perror("Failed to open AOF for writing");
            }
        }
    }

    ~DbSyncEngine() {
        if (aof_fd >= 0) {
            fsync(aof_fd);
            close(aof_fd);
        }
    }

    void recover_from_aof() {
        int fd = open(file_name.c_str(), O_RDONLY);
        if (fd < 0) {
            std::cout << "[Reactor " << reactor_id << "] No AOF file found, starting fresh." << std::endl;
            return;
        }

        // Get file size
        struct stat st;
        fstat(fd, &st);
        size_t file_size = st.st_size;
        
        if (file_size == 0) {
            close(fd);
            return;
        }

        // Read entire file
        std::vector<char> buffer(file_size);
        ssize_t bytes_read = read(fd, buffer.data(), file_size);
        close(fd);

        if (bytes_read <= 0) return;

        const char* data = buffer.data();
        size_t len = bytes_read;
        size_t pos = 0;
        size_t commands_recovered = 0;

        while (pos < len) {
            // Expect *N\r\n (array header)
            if (data[pos] != '*') break;
            
            size_t line_end = pos;
            while (line_end < len && data[line_end] != '\r') line_end++;
            if (line_end + 1 >= len) break;
            
            int num_args = 0;
            std::from_chars(data + pos + 1, data + line_end, num_args);
            pos = line_end + 2; // skip \r\n
            
            if (num_args < 1) break;

            // Parse command name
            std::string_view cmd_name = parse_bulk_string(data, len, pos);
            if (cmd_name.empty()) break;

            // Handle SET command: *3\r\n$3\r\nSET\r\n$keylen\r\nkey\r\n$vallen\r\nval\r\n
            if ((cmd_name == "SET" || cmd_name == "set") && num_args >= 3) {
                std::string_view key = parse_bulk_string(data, len, pos);
                std::string_view value = parse_bulk_string(data, len, pos);
                
                if (!key.empty() && !value.empty()) {
                    // Check for optional TTL args (PXAT, PX, EX)
                    uint64_t ttl_ms = 0;
                    if (num_args >= 5) {
                        std::string_view flag = parse_bulk_string(data, len, pos);
                        std::string_view timeout = parse_bulk_string(data, len, pos);
                        
                        uint64_t val = 0;
                        std::from_chars(timeout.data(), timeout.data() + timeout.size(), val);
                        
                        if (flag == "PXAT" || flag == "pxat") {
                            // Absolute timestamp - check if already expired
                            if (val > get_now()) {
                                ttl_ms = val - get_now();
                            } else {
                                // Already expired, skip this key
                                commands_recovered++;
                                continue;
                            }
                        } else if (flag == "PX" || flag == "px") {
                            ttl_ms = val;
                        } else if (flag == "EX" || flag == "ex") {
                            ttl_ms = val * 1000;
                        }
                    }
                    
                    set(key, value, ttl_ms);
                    commands_recovered++;
                }
            }
            // Skip other commands for now (just consume their args)
            else {
                for (int i = 1; i < num_args; i++) {
                    parse_bulk_string(data, len, pos);
                }
            }
        }

        std::cout << "[Reactor " << reactor_id << "] Recovered " << commands_recovered 
                  << " commands from " << file_name << std::endl;
    }
    
    uint64_t get_now(){
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    // Get local shard index for set/get operations (must own the key!)
    int get_shard_index(std::string_view key) const {
        return get_local_shard_index(key);
    }

    // The Set Operation
    int get_shard_count() const {
        return shard_count;
    }

    Shard& get_shard(int index) {
        return shards[index];
    }

    void set(std::string_view key, std::string_view value, uint64_t ttl_ms = 0) {
        int local_idx = get_local_shard_index(key);
        auto& s = shards[local_idx];
        uint64_t expiry = ttl_ms > 0 ? get_now() + ttl_ms : 0;
        s.data[std::string(key)] = {std::string(value), expiry};
    }

    // The Get Operation
    std::optional<std::string> get(std::string_view key) {
        int local_idx = get_local_shard_index(key);
        auto& s = shards[local_idx];
        auto it = s.data.find(std::string(key));
        if (it == s.data.end()) return std::nullopt;
        if (it->second.expires_at != 0 && it->second.expires_at < get_now()) {
            s.data.erase(std::string(key));
            return std::nullopt;
        }
        return it->second.data;
    }

    bool del(std::string_view key) {
        int local_idx = get_local_shard_index(key);
        auto& s = shards[local_idx];
        return s.data.erase(std::string(key)) > 0;
    }
};
