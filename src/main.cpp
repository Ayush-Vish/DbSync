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

#include "../include/dbsync_engine.hpp"
#include "../include/resp_parser.hpp"
#include "../include/debug.hpp"

const int PORT = 6379;
const int QUEUE_DEPTH = 4096;
const int MAX_CONN_PER_THREAD = 4096; // Size of our pre-allocated pool

enum class OpType
{
    ACCEPT,
    READ,
    WRITE
};

// Connection object is now a fixed-size POD to live in the pool
struct Connection
{
    int fd;
    OpType type;
    char buffer[4096];
    std::string response_data;

    // Reset helper to reuse the object without re-allocation
    void reset()
    {
        fd = -1;
        response_data.clear();
        // buffer doesn't need zeroing, read will overwrite it
    }
};

//

int create_shared_socket()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    sockaddr_in addr{AF_INET, htons(PORT), INADDR_ANY};
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        return -1;
    }
    listen(fd, 1024);
    return fd;
}
#include <sys/un.h>

int create_shared_socket1() {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    const char* path = "/tmp/dbsync.sock";
    unlink(path);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    chmod(path, 0777);
    listen(fd, 1024);
    return fd;
}
// ==========================================
// THE ZERO-ALLOCATION REACTOR
// ==========================================
void run_reactor(int reactor_id, int physical_core_id)
{
    // 1. HARDWARE ISOLATION
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(physical_core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);


    DbSyncEngine engine(512);

    // 2. OBJECT POOLING (Per-thread, zero locks)
    std::vector<Connection> conn_pool(MAX_CONN_PER_THREAD);
    std::stack<int> free_indices;
    for (int i = 0; i < MAX_CONN_PER_THREAD; ++i)
    {
        conn_pool[i].reset();
        free_indices.push(i);
    }

    auto get_conn = [&]() -> Connection *
    {
        if (free_indices.empty())
            return nullptr;
        int idx = free_indices.top();
        free_indices.pop();
        return &conn_pool[idx];
    };

    auto release_conn = [&](Connection *conn)
    {
        conn->reset();
        free_indices.push(conn - &conn_pool[0]);
    };

    // 3. IO_URING SETUP
    struct io_uring ring;
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
    int thread_socket = create_shared_socket();
    // int thread_socket = create_shared_socket1();
    if (thread_socket < 0)
    {
        std::cerr << "Reactor " << reactor_id << ": Failed to create socket." << std::endl;
        return;
    }

    auto submit_accept = [&]()
    {
        Connection *conn = get_conn();
        if (!conn)
            return;
        conn->fd = thread_socket;
        conn->type = OpType::ACCEPT;
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        io_uring_prep_accept(sqe, thread_socket, nullptr, nullptr, 0);
        io_uring_sqe_set_data(sqe, conn);
    };

    submit_accept();

    while (true)
    {
        struct io_uring_cqe *cqe;
        io_uring_submit(&ring);
        if (io_uring_wait_cqe(&ring, &cqe) < 0)
            continue;

        Connection *conn = (Connection *)io_uring_cqe_get_data(cqe);
        int res = cqe->res;

        if (res < 0)
        {
            if (conn->type != OpType::ACCEPT)
                release_conn(conn);
        }
        else if (conn->type == OpType::ACCEPT)
        {
            Connection *client = get_conn();
            if (client)
            {
                client->fd = res;
                client->type = OpType::READ;
                struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
                io_uring_prep_read(sqe, client->fd, client->buffer, 4096, 0);
                io_uring_sqe_set_data(sqe, client);
            }
            else
            {
                close(res);
            }
            submit_accept(); // Always keep one accept pending
        }
        else if (conn->type == OpType::READ)
        {
            if (res == 0)
            {
                close(conn->fd);
                release_conn(conn);
            }
            else
            {
                std::string_view remaining(conn->buffer, res);
                conn->response_data.clear();

                while (!remaining.empty())
                {
                    auto [cmd, consumed] = RespParser::parse(remaining);

                    if (consumed == 0)
                        break; // Buffer incomplete or corrupted

                    if (cmd.type == "SET" && cmd.args.size() >= 2)
                    {
                        uint64_t ttl_ms = 0;
                        if (cmd.args.size() >= 4)
                        {
                            std::string_view flag = cmd.args[2];
                            std::string_view timeout = cmd.args[3];
                            uint64_t val = 0;

                            // Fast numeric parsing (No exceptions)
                            auto [ptr, ec] = std::from_chars(timeout.data(), timeout.data() + timeout.size(), val);

                            if (ec == std::errc())
                            {
                                if (flag == "EX" || flag == "ex")
                                {
                                    ttl_ms = val * 1000; // Convert seconds to ms
                                }
                                else if (flag == "PX" || flag == "px")
                                {
                                    ttl_ms = val;
                                }
                            }
                        }
                        // Case: SET key value 2000 (3 args - custom non-standard behavior)
                        else if (cmd.args.size() == 3)
                        {
                            std::from_chars(cmd.args[2].data(), cmd.args[2].data() + cmd.args[2].size(), ttl_ms);
                        }

                        engine.set(cmd.args[0], cmd.args[1], ttl_ms);
                        conn->response_data.append("+OK\r\n");
                    }
                    else if (cmd.type == "GET" && !cmd.args.empty())
                    {
                        auto val = engine.get(cmd.args[0]);
                        if (val)
                        {
                            conn->response_data.append("$" + std::to_string(val->size()) + "\r\n" + *val + "\r\n");
                        }
                        else
                        {
                            conn->response_data.append("$-1\r\n");
                        }
                    }
                    else if (cmd.type == "EXPIRE" && cmd.args.size() >= 2)
                    {
                        uint64_t ttl_ms = 0;
                        auto arg = cmd.args[1];
                        std::from_chars(arg.data(), arg.data() + arg.size(), ttl_ms);

                        // TODO: Implement EXPIRE logic in DbSyncEngine
                        //  For now, just respond with 1 (success)

                        conn->response_data.append(":1\r\n");
                    }
                    else if (cmd.type == "PING")
                    {
                        conn->response_data.append("+PONG\r\n");
                    }
                    else
                    {
                        conn->response_data.append("-ERR unknown command\r\n");
                    }

                    remaining.remove_prefix(consumed);
                }

                if (!conn->response_data.empty())
                {
                    conn->type = OpType::WRITE;
                    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
                    io_uring_prep_send(sqe, conn->fd, conn->response_data.c_str(), conn->response_data.size(), 0);
                    io_uring_sqe_set_data(sqe, conn);
                }
                else
                {
                    // If we didn't get a full command, wait for more data
                    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
                    io_uring_prep_read(sqe, conn->fd, conn->buffer, 4096, 0);
                    io_uring_sqe_set_data(sqe, conn);
                }
            }
        }
        else if (conn->type == OpType::WRITE)
        {
            // Write complete, recycle for next read
            conn->type = OpType::READ;
            conn->response_data.clear();
            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
            io_uring_prep_read(sqe, conn->fd, conn->buffer, 4096, 0);
            io_uring_sqe_set_data(sqe, conn);
        }

        io_uring_cqe_seen(&ring, cqe);
    }
}

// void run_janitor(DbSyncEngine &engine)
// {
//     while (true)
//     {
//         for (int i = 0; i < engine.get_shard_count(); ++i)
//         {
//             auto &s = engine.get_shard(i);

//             // Try to lock without blocking the 4M RPS reactors
//             std::unique_lock lock(s.mtx, std::try_to_lock);
//             if (!lock.owns_lock() || s.data.empty())
//                 continue;

//             int expired_found = 0;
//             int sampled = 0;
//             auto it = s.data.begin();

//             // Random sampling: check up to 20 keys
//             while (sampled < 20 && it != s.data.end())
//             {
//                 if (it->second.expires_at > 0 && it->second.expires_at < engine.get_now())
//                 {
//                     // Capture current, then increment it BEFORE erasing
//                     auto current = it++;
//                     s.data.erase(current);
//                     expired_found++;
//                 }
//                 else
//                 {
//                     ++it;
//                 }
//                 sampled++;
//             }

//             // If the shard is "dirty" (>25% expired), we don't sleep
//             if (expired_found > 1)
//                 continue;
//         }
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
// }

int main()
{
    // Increase shards significantly to reduce thread collision
    // BG thread for expired key cleanup
    // std::thread janitor_thread(run_janitor, std::ref(engine));
    // janitor_thread.detach();

    std::cout << "🧹 Janitor active: Sampling shards for expired keys..." << std::endl;
    int logical_cores = std::thread::hardware_concurrency();
    // We target only physical cores (usually half of logical)
    int num_reactors = logical_cores / 2;
    if (num_reactors == 0)
        num_reactors = 1;

    std::vector<std::thread> threads;
    std::cout << "🔥 DbSync Phase 5: Zero-Allocation Multi-Reactor" << std::endl;
    std::cout << "🚀 Core Pinning: Using " << num_reactors << " Physical Cores" << std::endl;

    for (int i = 0; i < num_reactors; ++i)
    {
        int physical_core = i * 2; // Jump by 2 to skip Hyper-Thread siblings
        threads.emplace_back(run_reactor, i, physical_core);
    }

    for (auto &t : threads)
        t.join();
    return 0;
}
