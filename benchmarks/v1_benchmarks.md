### These are the benchmarks When using only one Socket for 50 clients.

```
GET: rps=0.0 (overall: 93.1) avg_msec=-nan (overa                                                 GET: rps=0.0 (overall: 92.9) avg_msec=-nan (overaGET: rps=0.0 (overall: 66.1) avg_msec=-nan (overall: 0.012) 
====== GET ======                                           
  10000 requests completed in 536.26 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.007 milliseconds (cumulative count 2127)
50.000% <= 0.015 milliseconds (cumulative count 7957)
87.500% <= 0.023 milliseconds (cumulative count 9869)
99.219% <= 0.031 milliseconds (cumulative count 9943)
99.609% <= 0.047 milliseconds (cumulative count 9961)
99.805% <= 0.087 milliseconds (cumulative count 9984)
99.902% <= 0.103 milliseconds (cumulative count 9991)
99.951% <= 0.183 milliseconds (cumulative count 9996)
99.976% <= 0.207 milliseconds (cumulative count 9998)
99.988% <= 0.231 milliseconds (cumulative count 9999)
99.994% <= 0.279 milliseconds (cumulative count 10000)
100.000% <= 0.279 milliseconds (cumulative count 10000)

Cumulative distribution of latencies:
99.910% <= 0.103 milliseconds (cumulative count 9991)
99.980% <= 0.207 milliseconds (cumulative count 9998)
100.000% <= 0.303 milliseconds (cumulative count 10000)

Summary:
  throughput summary: 18.65 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.012     0.000     0.015     0.023     0.031     0.279

```

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <charconv>
const int PORT = 6379;
// well this is the class in which is the main Engine of the DBSync server

class DbSyncEngine {
private:
    struct Shard{
        std::unordered_map<std::string,std::string> data;
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
    void set(const std::string& key, const std::string& value) {
        auto &s = shards[get_shard_index(key)];
        std::unique_lock lock(s.mtx);
        s.data[std::string(key)] = std::string(value);
    }
    // The Get Operation
    std::optional<std::string> get (const std::string&key   ) {
        auto &s = shards[get_shard_index(key)];
        std::shared_lock lock(s.mtx);
        auto it = s.data.find(key);
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





struct Command{
    std::string_view type; // used string  view to avoid copying strings , Ex: SET, GET
    std::vector<std::string_view> args; // Ex: SET key value -> args = [key,value]
};

class RespParser {
public:
    static Command parse(std::string_view buffer) {
        // Redis commands start with '*' if they are an array (which most are)
        if (buffer.empty() || buffer[0] != '*') return {"UNKNOWN", {}};

        size_t pos = 0;
        // This helper lambda finds the next line ending (\r\n)
        auto read_line = [&]() -> std::string_view {
            size_t start = pos;
            size_t end = buffer.find("\r\n", pos);
            if (end == std::string_view::npos) return "";
            pos = end + 2;
            return buffer.substr(start, end - start);
        };

        std::string_view header = read_line(); // This is the "*3" line
        int num_args = 0;
        // Extracting the number after the '*'
        std::from_chars(header.data() + 1, header.data() + header.size(), num_args);

        Command cmd;
        for (int i = 0; i < num_args; ++i) {
            read_line(); // Skip the "$3" (length) lines, we don't strictly need them for this simple version
            std::string_view arg = read_line(); // This is the actual word (SET, key, or value)
            if (i == 0) cmd.type = arg;
            else cmd.args.push_back(arg);
        }
        return cmd;
    }
};

int main (){
    DbSyncEngine engine(16); // create an engine with 16 shards
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 128) < 0) {
        perror("listen");
        return 1;
    }

    std::cout << "DBSync server is listening on port " << PORT << "..." << std::endl;

    while(true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        std::cout << "Accepted connection on socket: " << client_fd << std::endl;
        if(client_fd < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }
        while(true) {

            char buffer[1024] = {0};
            ssize_t bytes_read = read(client_fd, buffer, 1024);
            if(bytes_read <= 0) {
                close(client_fd);
                break;;
            }
            std::string_view raw_request(buffer, bytes_read);
            Command cmd = RespParser::parse(raw_request);
            if(cmd.type == "SET" && cmd.args.size() >= 2 ) {
                engine.set(std::string(cmd.args[0]), std::string(cmd.args[1]));
                const char *response = "+OK\r\n";
                send(client_fd, response, strlen(response), 0);
            }else if( cmd.type == "GET" && cmd.args.size() >= 1) {
                auto val = engine.get(std::string(cmd.args[0]));
                if(val){
                    std::string res = "$" + std::to_string(val->size()) + "\r\n" + *val + "\r\n";
                    send(client_fd, res.c_str(), res.size(), 0);
                }else{
                    const char *response = "$-1\r\n"; // Null bulk string
                    send(client_fd, response, strlen(response), 0);
                }
            }else if( cmd.type == "DEL" && cmd.args.size() >= 1) {
                bool deleted = engine.del(cmd.args[0]);
                std::string res = ":" + std::to_string(deleted ? 1 : 0) + "\r\n";
                send(client_fd, res.c_str(), res.size(), 0);
            }else{
                const char *response = "-ERR unknown command\r\n";
                send(client_fd, response, strlen(response), 0);
            }
        }
        close(client_fd);

    }
    close(server_fd);
    return 0;
    
}
```

```bash
{13:59}~/Desktop/DB:master ✓ ➭ 
{14:00}~/Desktop/DB:master ✓ ➭ redis-benchmark -p 6379 -t set,get -n 10000 -c 50~
```
