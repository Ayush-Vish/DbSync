
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
    int shard_count;
    int reactor_id;
    std::string file_name;

    int get_shard_index(const std::string_view& key) {
        return std::hash<std::string_view>{}(key) % shard_count;
    }

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

    DbSyncEngine(int num_shards, int reactor_id = -1) 
        : shard_count(num_shards), shards(num_shards), reactor_id(reactor_id) {
        
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
