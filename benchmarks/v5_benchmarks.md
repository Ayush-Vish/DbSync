{19:25}~/Desktop/DB/src:main ✗ ➭ # Use --threads to make the benchmark tool parallel
redis-benchmark -p 6379 -t set,get -n 1000000 -c 100 --threads 8
WARNING: Could not fetch server CONFIG
====== SET ======                                                     
  1000000 requests completed in 3.26 seconds
  100 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: yes
  threads: 8

Latency by percentile distribution:
0.000% <= 0.015 milliseconds (cumulative count 115)
50.000% <= 0.071 milliseconds (cumulative count 559108)
75.000% <= 0.263 milliseconds (cumulative count 751384)
87.500% <= 0.503 milliseconds (cumulative count 875766)
93.750% <= 0.663 milliseconds (cumulative count 938142)
96.875% <= 0.767 milliseconds (cumulative count 969653)
98.438% <= 0.871 milliseconds (cumulative count 984985)
99.219% <= 1.135 milliseconds (cumulative count 992189)
99.609% <= 3.535 milliseconds (cumulative count 996121)
99.805% <= 4.479 milliseconds (cumulative count 998058)
99.902% <= 5.439 milliseconds (cumulative count 999027)
99.951% <= 6.583 milliseconds (cumulative count 999515)
99.976% <= 7.407 milliseconds (cumulative count 999756)
99.988% <= 7.951 milliseconds (cumulative count 999879)
99.994% <= 9.447 milliseconds (cumulative count 999939)
99.997% <= 11.703 milliseconds (cumulative count 999970)
99.998% <= 15.999 milliseconds (cumulative count 999985)
99.999% <= 18.479 milliseconds (cumulative count 999994)
100.000% <= 18.591 milliseconds (cumulative count 999998)
100.000% <= 18.831 milliseconds (cumulative count 999999)
100.000% <= 21.775 milliseconds (cumulative count 1000000)
100.000% <= 21.775 milliseconds (cumulative count 1000000)

Cumulative distribution of latencies:
68.951% <= 0.103 milliseconds (cumulative count 689510)
73.515% <= 0.207 milliseconds (cumulative count 735147)
76.409% <= 0.303 milliseconds (cumulative count 764089)
81.627% <= 0.407 milliseconds (cumulative count 816270)
87.577% <= 0.503 milliseconds (cumulative count 875766)
91.799% <= 0.607 milliseconds (cumulative count 917986)
95.189% <= 0.703 milliseconds (cumulative count 951885)
97.749% <= 0.807 milliseconds (cumulative count 977485)
98.730% <= 0.903 milliseconds (cumulative count 987299)
99.099% <= 1.007 milliseconds (cumulative count 990992)
99.202% <= 1.103 milliseconds (cumulative count 992023)
99.251% <= 1.207 milliseconds (cumulative count 992512)
99.278% <= 1.303 milliseconds (cumulative count 992776)
99.299% <= 1.407 milliseconds (cumulative count 992990)
99.316% <= 1.503 milliseconds (cumulative count 993158)
99.330% <= 1.607 milliseconds (cumulative count 993297)
99.343% <= 1.703 milliseconds (cumulative count 993425)
99.355% <= 1.807 milliseconds (cumulative count 993551)
99.368% <= 1.903 milliseconds (cumulative count 993679)
99.383% <= 2.007 milliseconds (cumulative count 993829)
99.396% <= 2.103 milliseconds (cumulative count 993964)
99.526% <= 3.103 milliseconds (cumulative count 995258)
99.743% <= 4.103 milliseconds (cumulative count 997425)
99.888% <= 5.103 milliseconds (cumulative count 998880)
99.934% <= 6.103 milliseconds (cumulative count 999336)
99.966% <= 7.103 milliseconds (cumulative count 999659)
99.989% <= 8.103 milliseconds (cumulative count 999892)
99.994% <= 9.103 milliseconds (cumulative count 999937)
99.995% <= 10.103 milliseconds (cumulative count 999946)
99.996% <= 11.103 milliseconds (cumulative count 999965)
99.997% <= 12.103 milliseconds (cumulative count 999972)
99.998% <= 14.103 milliseconds (cumulative count 999976)
99.999% <= 16.103 milliseconds (cumulative count 999985)
99.999% <= 18.111 milliseconds (cumulative count 999989)
100.000% <= 19.103 milliseconds (cumulative count 999999)
100.000% <= 22.111 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 306372.56 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.206     0.008     0.071     0.703     0.967    21.775
====== GET ======                                                     
  1000000 requests completed in 3.02 seconds
  100 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: yes
  threads: 8

Latency by percentile distribution:
0.000% <= 0.015 milliseconds (cumulative count 99)
50.000% <= 0.071 milliseconds (cumulative count 550772)
75.000% <= 0.215 milliseconds (cumulative count 750908)
87.500% <= 0.511 milliseconds (cumulative count 876499)
93.750% <= 0.663 milliseconds (cumulative count 940361)
96.875% <= 0.767 milliseconds (cumulative count 968886)
98.438% <= 0.871 milliseconds (cumulative count 984617)
99.219% <= 0.991 milliseconds (cumulative count 992352)
99.609% <= 2.167 milliseconds (cumulative count 996096)
99.805% <= 3.919 milliseconds (cumulative count 998052)
99.902% <= 4.695 milliseconds (cumulative count 999030)
99.951% <= 6.455 milliseconds (cumulative count 999513)
99.976% <= 7.639 milliseconds (cumulative count 999758)
99.988% <= 8.943 milliseconds (cumulative count 999878)
99.994% <= 12.287 milliseconds (cumulative count 999939)
99.997% <= 17.631 milliseconds (cumulative count 999970)
99.998% <= 18.719 milliseconds (cumulative count 999985)
99.999% <= 25.215 milliseconds (cumulative count 999993)
100.000% <= 25.503 milliseconds (cumulative count 999997)
100.000% <= 25.663 milliseconds (cumulative count 999999)
100.000% <= 26.335 milliseconds (cumulative count 1000000)
100.000% <= 26.335 milliseconds (cumulative count 1000000)

Cumulative distribution of latencies:
68.763% <= 0.103 milliseconds (cumulative count 687631)
74.725% <= 0.207 milliseconds (cumulative count 747254)
78.582% <= 0.303 milliseconds (cumulative count 785815)
83.001% <= 0.407 milliseconds (cumulative count 830015)
87.303% <= 0.503 milliseconds (cumulative count 873032)
91.872% <= 0.607 milliseconds (cumulative count 918718)
95.349% <= 0.703 milliseconds (cumulative count 953494)
97.599% <= 0.807 milliseconds (cumulative count 975992)
98.759% <= 0.903 milliseconds (cumulative count 987591)
99.279% <= 1.007 milliseconds (cumulative count 992794)
99.414% <= 1.103 milliseconds (cumulative count 994144)
99.461% <= 1.207 milliseconds (cumulative count 994611)
99.486% <= 1.303 milliseconds (cumulative count 994861)
99.504% <= 1.407 milliseconds (cumulative count 995041)
99.520% <= 1.503 milliseconds (cumulative count 995203)
99.533% <= 1.607 milliseconds (cumulative count 995327)
99.546% <= 1.703 milliseconds (cumulative count 995456)
99.560% <= 1.807 milliseconds (cumulative count 995596)
99.570% <= 1.903 milliseconds (cumulative count 995702)
99.585% <= 2.007 milliseconds (cumulative count 995847)
99.600% <= 2.103 milliseconds (cumulative count 996002)
99.713% <= 3.103 milliseconds (cumulative count 997127)
99.825% <= 4.103 milliseconds (cumulative count 998245)
99.926% <= 5.103 milliseconds (cumulative count 999260)
99.945% <= 6.103 milliseconds (cumulative count 999454)
99.963% <= 7.103 milliseconds (cumulative count 999633)
99.982% <= 8.103 milliseconds (cumulative count 999823)
99.988% <= 9.103 milliseconds (cumulative count 999884)
99.992% <= 10.103 milliseconds (cumulative count 999918)
99.993% <= 11.103 milliseconds (cumulative count 999931)
99.994% <= 12.103 milliseconds (cumulative count 999937)
99.995% <= 13.103 milliseconds (cumulative count 999947)
99.995% <= 14.103 milliseconds (cumulative count 999950)
99.996% <= 17.103 milliseconds (cumulative count 999962)
99.998% <= 18.111 milliseconds (cumulative count 999979)
99.999% <= 19.103 milliseconds (cumulative count 999988)
99.999% <= 20.111 milliseconds (cumulative count 999989)
99.999% <= 21.103 milliseconds (cumulative count 999990)
99.999% <= 25.103 milliseconds (cumulative count 999991)
100.000% <= 26.111 milliseconds (cumulative count 999999)
100.000% <= 27.103 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 331235.50 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.197     0.008     0.071     0.695     0.943    26.335

{19:26}~/Desktop/DB/src:main ✗ ➭ 



```

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <stack>
#include <thread>
#include <liburing.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

// Assuming these are in your include folder
#include "../include/dbsync_engine.hpp"
#include "../include/resp_parser.hpp"
#include "../include/debug.hpp"

const int PORT = 6379;
const int QUEUE_DEPTH = 4096;
const int MAX_CONN_PER_THREAD = 4096; // Size of our pre-allocated pool

enum class OpType { ACCEPT, READ, WRITE };

// Connection object is now a fixed-size POD to live in the pool
struct Connection {
    int fd;
    OpType type;
    char buffer[4096];
    std::string response_data;

    // Reset helper to reuse the object without re-allocation
    void reset() {
        fd = -1;
        response_data.clear();
        // buffer doesn't need zeroing, read will overwrite it
    }
};

// 

int create_shared_socket() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    sockaddr_in addr{AF_INET, htons(PORT), INADDR_ANY};
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return -1;
    }
    listen(fd, 1024);
    return fd;
}

// ==========================================
// THE ZERO-ALLOCATION REACTOR
// ==========================================
void run_reactor(int reactor_id, int physical_core_id, DbSyncEngine& engine) {
    // 1. HARDWARE ISOLATION
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(physical_core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    // 2. OBJECT POOLING (Per-thread, zero locks)
    std::vector<Connection> conn_pool(MAX_CONN_PER_THREAD);
    std::stack<int> free_indices;
    for (int i = 0; i < MAX_CONN_PER_THREAD; ++i) {
        conn_pool[i].reset();
        free_indices.push(i);
    }

    auto get_conn = [&]() -> Connection* {
        if (free_indices.empty()) return nullptr;
        int idx = free_indices.top();
        free_indices.pop();
        return &conn_pool[idx];
    };

    auto release_conn = [&](Connection* conn) {
        conn->reset();
        free_indices.push(conn - &conn_pool[0]);
    };

    // 3. IO_URING SETUP
    struct io_uring ring;
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
    int thread_socket = create_shared_socket();

    auto submit_accept = [&]() {
        Connection* conn = get_conn();
        if (!conn) return;
        conn->fd = thread_socket;
        conn->type = OpType::ACCEPT;
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
        io_uring_prep_accept(sqe, thread_socket, nullptr, nullptr, 0);
        io_uring_sqe_set_data(sqe, conn);
    };

    submit_accept();

    while (true) {
        struct io_uring_cqe* cqe;
        io_uring_submit(&ring);
        if (io_uring_wait_cqe(&ring, &cqe) < 0) continue;

        Connection* conn = (Connection*)io_uring_cqe_get_data(cqe);
        int res = cqe->res;

        if (res < 0) {
            if (conn->type != OpType::ACCEPT) release_conn(conn);
        } else if (conn->type == OpType::ACCEPT) {
            Connection* client = get_conn();
            if (client) {
                client->fd = res;
                client->type = OpType::READ;
                struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
                io_uring_prep_read(sqe, client->fd, client->buffer, 4096, 0);
                io_uring_sqe_set_data(sqe, client);
            } else {
                close(res);
            }
            submit_accept(); // Always keep one accept pending
        } else if (conn->type == OpType::READ) {
            if (res == 0) {
                close(conn->fd);
                release_conn(conn);
            } else {
                std::string_view raw(conn->buffer, res);
                Command cmd = RespParser::parse(raw);
                
                std::string response;
                if (cmd.type == "SET" && cmd.args.size() >= 2) {
                    engine.set(cmd.args[0], cmd.args[1]);
                    response = "+OK\r\n";
                } else if (cmd.type == "GET" && !cmd.args.empty()) {
                    auto val = engine.get(cmd.args[0]);
                    response = val ? "$" + std::to_string(val->size()) + "\r\n" + *val + "\r\n" : "$-1\r\n";
                } else {
                    response = "+PONG\r\n";
                }

                conn->type = OpType::WRITE;
                conn->response_data = std::move(response);
                struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
                io_uring_prep_send(sqe, conn->fd, conn->response_data.c_str(), conn->response_data.size(), 0);
                io_uring_sqe_set_data(sqe, conn);
            }
        } else if (conn->type == OpType::WRITE) {
            // Write complete, recycle for next read
            conn->type = OpType::READ;
            conn->response_data.clear();
            struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
            io_uring_prep_read(sqe, conn->fd, conn->buffer, 4096, 0);
            io_uring_sqe_set_data(sqe, conn);
        }

        io_uring_cqe_seen(&ring, cqe);
    }
}

int main() {
    // Increase shards significantly to reduce thread collision
    DbSyncEngine engine(2048); 
    
    int logical_cores = std::thread::hardware_concurrency();
    // We target only physical cores (usually half of logical)
    int num_reactors = logical_cores / 2;
    if (num_reactors == 0) num_reactors = 1;

    std::vector<std::thread> threads;
    std::cout << "🔥 DbSync Phase 5: Zero-Allocation Multi-Reactor" << std::endl;
    std::cout << "🚀 Core Pinning: Using " << num_reactors << " Physical Cores" << std::endl;

    for (int i = 0; i < num_reactors; ++i) {
        int physical_core = i * 2; // Jump by 2 to skip Hyper-Thread siblings
        threads.emplace_back(run_reactor, i, physical_core, std::ref(engine));
    }

    for (auto& t : threads) t.join();
    return 0;
}

```
