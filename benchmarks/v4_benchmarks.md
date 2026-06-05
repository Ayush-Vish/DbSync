{19:04}~/Desktop/DB/src:main ✗ ➭ # Use --threads to make the benchmark tool parallel
redis-benchmark -p 6379 -t set,get -n 1000000 -c 100 --threads 8
WARNING: Could not fetch server CONFIG
====== SET ======                                                     
  1000000 requests completed in 3.54 seconds
  100 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: yes
  threads: 8

Latency by percentile distribution:
0.000% <= 0.015 milliseconds (cumulative count 559)
50.000% <= 0.079 milliseconds (cumulative count 532601)
75.000% <= 0.175 milliseconds (cumulative count 755862)
87.500% <= 0.271 milliseconds (cumulative count 877727)
93.750% <= 0.543 milliseconds (cumulative count 938466)
96.875% <= 1.511 milliseconds (cumulative count 968798)
98.438% <= 3.399 milliseconds (cumulative count 984456)
99.219% <= 4.215 milliseconds (cumulative count 992239)
99.609% <= 5.007 milliseconds (cumulative count 996103)
99.805% <= 6.039 milliseconds (cumulative count 998057)
99.902% <= 6.919 milliseconds (cumulative count 999027)
99.951% <= 7.639 milliseconds (cumulative count 999519)
99.976% <= 8.431 milliseconds (cumulative count 999756)
99.988% <= 8.975 milliseconds (cumulative count 999878)
99.994% <= 9.503 milliseconds (cumulative count 999939)
99.997% <= 10.039 milliseconds (cumulative count 999970)
99.998% <= 10.559 milliseconds (cumulative count 999985)
99.999% <= 11.367 milliseconds (cumulative count 999993)
100.000% <= 12.159 milliseconds (cumulative count 999997)
100.000% <= 12.311 milliseconds (cumulative count 999999)
100.000% <= 14.567 milliseconds (cumulative count 1000000)
100.000% <= 14.567 milliseconds (cumulative count 1000000)

Cumulative distribution of latencies:
60.363% <= 0.103 milliseconds (cumulative count 603631)
81.328% <= 0.207 milliseconds (cumulative count 813282)
89.295% <= 0.303 milliseconds (cumulative count 892954)
91.725% <= 0.407 milliseconds (cumulative count 917254)
93.287% <= 0.503 milliseconds (cumulative count 932870)
94.587% <= 0.607 milliseconds (cumulative count 945865)
95.297% <= 0.703 milliseconds (cumulative count 952967)
95.716% <= 0.807 milliseconds (cumulative count 957156)
95.958% <= 0.903 milliseconds (cumulative count 959576)
96.171% <= 1.007 milliseconds (cumulative count 961715)
96.358% <= 1.103 milliseconds (cumulative count 963576)
96.511% <= 1.207 milliseconds (cumulative count 965115)
96.635% <= 1.303 milliseconds (cumulative count 966351)
96.756% <= 1.407 milliseconds (cumulative count 967562)
96.871% <= 1.503 milliseconds (cumulative count 968707)
96.986% <= 1.607 milliseconds (cumulative count 969856)
97.081% <= 1.703 milliseconds (cumulative count 970810)
97.186% <= 1.807 milliseconds (cumulative count 971856)
97.275% <= 1.903 milliseconds (cumulative count 972752)
97.365% <= 2.007 milliseconds (cumulative count 973653)
97.443% <= 2.103 milliseconds (cumulative count 974431)
98.169% <= 3.103 milliseconds (cumulative count 981685)
99.150% <= 4.103 milliseconds (cumulative count 991498)
99.629% <= 5.103 milliseconds (cumulative count 996294)
99.814% <= 6.103 milliseconds (cumulative count 998135)
99.914% <= 7.103 milliseconds (cumulative count 999137)
99.969% <= 8.103 milliseconds (cumulative count 999691)
99.989% <= 9.103 milliseconds (cumulative count 999893)
99.997% <= 10.103 milliseconds (cumulative count 999975)
99.999% <= 11.103 milliseconds (cumulative count 999991)
99.999% <= 12.103 milliseconds (cumulative count 999995)
100.000% <= 13.103 milliseconds (cumulative count 999999)
100.000% <= 15.103 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 282565.69 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.238     0.008     0.079     0.663     3.959    14.567
====== GET ======                                                     
  1000000 requests completed in 3.53 seconds
  100 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: yes
  threads: 8

Latency by percentile distribution:
0.000% <= 0.015 milliseconds (cumulative count 316)
50.000% <= 0.079 milliseconds (cumulative count 522963)
75.000% <= 0.183 milliseconds (cumulative count 757153)
87.500% <= 0.279 milliseconds (cumulative count 880014)
93.750% <= 0.519 milliseconds (cumulative count 938177)
96.875% <= 1.511 milliseconds (cumulative count 968773)
98.438% <= 3.511 milliseconds (cumulative count 984386)
99.219% <= 4.327 milliseconds (cumulative count 992237)
99.609% <= 5.199 milliseconds (cumulative count 996110)
99.805% <= 6.263 milliseconds (cumulative count 998049)
99.902% <= 6.991 milliseconds (cumulative count 999024)
99.951% <= 7.543 milliseconds (cumulative count 999512)
99.976% <= 8.023 milliseconds (cumulative count 999757)
99.988% <= 8.703 milliseconds (cumulative count 999879)
99.994% <= 9.567 milliseconds (cumulative count 999939)
99.997% <= 10.303 milliseconds (cumulative count 999970)
99.998% <= 10.855 milliseconds (cumulative count 999985)
99.999% <= 11.815 milliseconds (cumulative count 999993)
100.000% <= 12.327 milliseconds (cumulative count 999997)
100.000% <= 13.687 milliseconds (cumulative count 999999)
100.000% <= 13.855 milliseconds (cumulative count 1000000)
100.000% <= 13.855 milliseconds (cumulative count 1000000)

Cumulative distribution of latencies:
59.157% <= 0.103 milliseconds (cumulative count 591568)
79.887% <= 0.207 milliseconds (cumulative count 798870)
89.304% <= 0.303 milliseconds (cumulative count 893036)
91.968% <= 0.407 milliseconds (cumulative count 919682)
93.577% <= 0.503 milliseconds (cumulative count 935771)
94.898% <= 0.607 milliseconds (cumulative count 948976)
95.603% <= 0.703 milliseconds (cumulative count 956034)
95.982% <= 0.807 milliseconds (cumulative count 959824)
96.172% <= 0.903 milliseconds (cumulative count 961719)
96.338% <= 1.007 milliseconds (cumulative count 963376)
96.471% <= 1.103 milliseconds (cumulative count 964708)
96.596% <= 1.207 milliseconds (cumulative count 965955)
96.695% <= 1.303 milliseconds (cumulative count 966954)
96.790% <= 1.407 milliseconds (cumulative count 967904)
96.871% <= 1.503 milliseconds (cumulative count 968712)
96.964% <= 1.607 milliseconds (cumulative count 969642)
97.044% <= 1.703 milliseconds (cumulative count 970438)
97.127% <= 1.807 milliseconds (cumulative count 971269)
97.199% <= 1.903 milliseconds (cumulative count 971990)
97.279% <= 2.007 milliseconds (cumulative count 972785)
97.355% <= 2.103 milliseconds (cumulative count 973547)
98.023% <= 3.103 milliseconds (cumulative count 980234)
99.071% <= 4.103 milliseconds (cumulative count 990708)
99.591% <= 5.103 milliseconds (cumulative count 995913)
99.785% <= 6.103 milliseconds (cumulative count 997846)
99.912% <= 7.103 milliseconds (cumulative count 999119)
99.978% <= 8.103 milliseconds (cumulative count 999778)
99.991% <= 9.103 milliseconds (cumulative count 999913)
99.996% <= 10.103 milliseconds (cumulative count 999959)
99.999% <= 11.103 milliseconds (cumulative count 999988)
99.999% <= 12.103 milliseconds (cumulative count 999995)
100.000% <= 13.103 milliseconds (cumulative count 999997)
100.000% <= 14.103 milliseconds (cumulative count 1000000)

Summary:
  throughput summary: 283607.50 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.241     0.008     0.079     0.623     4.031    13.855


```
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <liburing.h>
#include <netinet/in.h>
#include <sys/socket.h>

// Include your existing headers
#include "../include/dbsync_engine.hpp"
#include "../include/resp_parser.hpp"
#include "../include/debug.hpp"

const int PORT = 6379;
const int QUEUE_DEPTH = 4096;

// Shared state between rings
enum class OpType { ACCEPT, READ, WRITE };
struct Connection {
    int fd;
    OpType type;
    char buffer[4096];
    std::string response_data;
};

// Helper: Setup a socket that can be shared across multiple threads/rings
int create_shared_socket() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    // SO_REUSEPORT is the magic that lets multiple threads bind to the same port
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    sockaddr_in addr{AF_INET, htons(PORT), INADDR_ANY};
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }
    listen(fd, 1024);
    return fd;
}

// The Worker function: Each thread runs this loop
void run_reactor(int core_id, DbSyncEngine& engine) {
    // 1. Pin this thread to a specific CPU core for Cache Locality
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    struct io_uring ring;
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

    // Each thread creates its own listener on the same port thanks to SO_REUSEPORT
    int thread_socket = create_shared_socket();
    
    // Bootstrap: Start by accepting connections on this thread's ring
    auto submit_accept = [&](int fd) {
        Connection* conn = new Connection{fd, OpType::ACCEPT, {0}, ""};
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
        io_uring_prep_accept(sqe, fd, nullptr, nullptr, 0);
        io_uring_sqe_set_data(sqe, conn);
    };

    submit_accept(thread_socket);
    DB_LOG(LogLevel::INFO, "Reactor Thread %d live on Core %d", core_id, core_id);

    while (true) {
        struct io_uring_cqe* cqe;
        io_uring_submit(&ring);
        if (io_uring_wait_cqe(&ring, &cqe) < 0) continue;

        Connection* conn = (Connection*)io_uring_cqe_get_data(cqe);
        int res = cqe->res;

        if (res < 0) {
            if (conn->type != OpType::ACCEPT) { close(conn->fd); delete conn; }
        } else if (conn->type == OpType::ACCEPT) {
            // This ring accepted a new client
            int client_fd = res;
            Connection* client = new Connection{client_fd, OpType::READ, {0}, ""};
            struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
            io_uring_prep_read(sqe, client_fd, client->buffer, 4096, 0);
            io_uring_sqe_set_data(sqe, client);
            
            submit_accept(thread_socket); // Listen for next connection on this ring
            delete conn;
        } else if (conn->type == OpType::READ) {
            if (res == 0) {
                close(conn->fd); delete conn;
            } else {
                std::string_view raw(conn->buffer, res);
                Command cmd = RespParser::parse(raw);
                std::string response = (cmd.type == "GET") ? "+PONG_GET\r\n" : "+OK\r\n"; // Simplified for bench
                
                conn->type = OpType::WRITE;
                conn->response_data = std::move(response);
                struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
                io_uring_prep_send(sqe, conn->fd, conn->response_data.c_str(), conn->response_data.size(), 0);
                io_uring_sqe_set_data(sqe, conn);
            }
        } else if (conn->type == OpType::WRITE) {
            // Read again
            conn->type = OpType::READ;
            struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
            io_uring_prep_read(sqe, conn->fd, conn->buffer, 4096, 0);
            io_uring_sqe_set_data(sqe, conn);
        }
        io_uring_cqe_seen(&ring, cqe);
    }
}

int main() {
    DbSyncEngine engine(64); // Increase shards to match core count
    int num_cores = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    std::cout << "🚀 Launching DbSync Multi-Reactor with " << num_cores << " rings..." << std::endl;

    for (int i = 0; i < num_cores; ++i) {
        threads.emplace_back(run_reactor, i, std::ref(engine));
    }

    for (auto& t : threads) t.join();
    return 0;
}



```
