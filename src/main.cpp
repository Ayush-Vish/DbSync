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
#include <charconv>

#include "../include/dbsync_engine.hpp"
#include "../include/resp_parser.hpp"
#include "../include/debug.hpp"

const int PORT = 6379;
const int QUEUE_DEPTH = 4096;
const int MAX_CONN_PER_THREAD = 4096;

enum class OpType
{
    ACCEPT,
    READ,
    WRITE,
    AOF_WRITE
};

struct Connection
{
    int fd;
    OpType type;
    char buffer[4096];
    std::string response_data;

    // AOF buffer
    char aof_buf[1024];
    uint32_t aof_len = 0;

    /**
     * @brief Reset connection runtime state to its initial (unused) values.
     *
     * Clears outgoing response data, marks the file descriptor as invalid, and
     * resets the staged AOF buffer length to zero.
     */
    void reset()
    {
        fd = -1;
        response_data.clear();
        aof_len = 0;
    }
};

/**
 * @brief Create and return a TCP listening socket bound to the configured PORT on all interfaces.
 *
 * The socket is configured for address and port reuse and is placed into listening state.
 *
 * @return int File descriptor of the listening socket on success, `-1` on failure.
 */
int create_shared_socket()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Bind failed");
        return -1;
    }
    listen(fd, 1024);
    return fd;
}

/**
 * @brief Starts and runs a per-reactor event loop pinned to a physical core.
 *
 * Initializes a per-reactor DbSyncEngine and an io_uring-based reactor that accepts connections,
 * performs asynchronous reads/writes, parses RESP commands (GET/SET/EXPIRE/PING/CONFIG), persists
 * SET operations to the reactor's AOF before replying, and manages a fixed-size connection pool.
 *
 * @param reactor_id Logical reactor identifier used to scope the per-reactor AOF and engine.
 * @param physical_core_id Physical CPU core index to which this reactor thread will be pinned.
 */
void run_reactor(int reactor_id, int physical_core_id)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(physical_core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    DbSyncEngine engine(512, reactor_id);  // Pass reactor_id for per-reactor AOF

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

    struct io_uring ring;
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
    int thread_socket = create_shared_socket();
    
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
            submit_accept();
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
                bool has_set_command = false;

                while (!remaining.empty())
                {
                    auto [cmd, consumed] = RespParser::parse(remaining);

                    if (consumed == 0)
                        break;

                    if (cmd.type == "SET" && cmd.args.size() >= 2)
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
                                {
                                    ttl_ms = val * 1000;
                                }
                                else if (flag == "PX" || flag == "px")
                                {
                                    ttl_ms = val;
                                }
                            }
                        }
                        else if (cmd.args.size() == 3)
                        {
                            std::from_chars(cmd.args[2].data(), cmd.args[2].data() + cmd.args[2].size(), ttl_ms);
                        }

                        engine.set(cmd.args[0], cmd.args[1], ttl_ms);
                        
                        // Prepare AOF
                        conn->aof_len = snprintf(
                            conn->aof_buf, sizeof(conn->aof_buf),
                            "*3\r\n$3\r\nSET\r\n$%zu\r\n%.*s\r\n$%zu\r\n%.*s\r\n",
                            cmd.args[0].size(), (int)cmd.args[0].size(), cmd.args[0].data(),
                            cmd.args[1].size(), (int)cmd.args[1].size(), cmd.args[1].data());
                        
                        has_set_command = true;
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
                    else if (cmd.type == "CONFIG")
                    {
                        conn->response_data.append("*0\r\n");
                    }
                    else if (cmd.type == "EXPIRE" && cmd.args.size() >= 2)
                    {
                        uint64_t ttl_ms = 0;
                        auto arg = cmd.args[1];
                        std::from_chars(arg.data(), arg.data() + arg.size(), ttl_ms);
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

                if (has_set_command && conn->aof_len > 0)
                {
                    // Write to AOF first
                    conn->type = OpType::AOF_WRITE;
                    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
                    io_uring_prep_write(sqe, engine.aof_fd, conn->aof_buf, conn->aof_len, -1);
                    sqe->flags |= IOSQE_ASYNC;
                    io_uring_sqe_set_data(sqe, conn);
                }
                else if (!conn->response_data.empty())
                {
                    conn->type = OpType::WRITE;
                    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
                    io_uring_prep_send(sqe, conn->fd, conn->response_data.c_str(), conn->response_data.size(), 0);
                    io_uring_sqe_set_data(sqe, conn);
                }
                else
                {
                    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
                    io_uring_prep_read(sqe, conn->fd, conn->buffer, 4096, 0);
                    io_uring_sqe_set_data(sqe, conn);
                }
            }
        }
        else if (conn->type == OpType::AOF_WRITE)
        {
            conn->aof_len = 0;
            
            // Now send response to client
            if (!conn->response_data.empty())
            {
                conn->type = OpType::WRITE;
                struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
                io_uring_prep_send(sqe, conn->fd, conn->response_data.c_str(), conn->response_data.size(), 0);
                io_uring_sqe_set_data(sqe, conn);
            }
            else
            {
                conn->type = OpType::READ;
                struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
                io_uring_prep_read(sqe, conn->fd, conn->buffer, 4096, 0);
                io_uring_sqe_set_data(sqe, conn);
            }
        }
        else if (conn->type == OpType::WRITE)
        {
            conn->type = OpType::READ;
            conn->response_data.clear();
            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
            io_uring_prep_read(sqe, conn->fd, conn->buffer, 4096, 0);
            io_uring_sqe_set_data(sqe, conn);
        }

        io_uring_cqe_seen(&ring, cqe);
    }
}

/**
 * @brief Program entry that starts per-reactor event loops and pins each reactor thread to a physical core.
 *
 * The function determines the number of reactor threads as half the available logical cores (minimum 1),
 * launches each reactor via run_reactor(reacator_id, physical_core_id) with a physical core stride of 2,
 * and joins all reactor threads before exiting.
 *
 * @return int 0 on successful termination.
 */
int main()
{
    std::cout << "🧹 Janitor active: Sampling shards for expired keys..." << std::endl;
    int logical_cores = std::thread::hardware_concurrency();
    int num_reactors = logical_cores / 2;
    if (num_reactors == 0)
        num_reactors = 1;

    std::vector<std::thread> threads;
    std::cout << "🔥 DbSync Phase 5: Zero-Allocation Multi-Reactor" << std::endl;
    std::cout << "🚀 Core Pinning: Using " << num_reactors << " Physical Cores" << std::endl;

    for (int i = 0; i < num_reactors; ++i)
    {
        int physical_core = i * 2;
        threads.emplace_back(run_reactor, i, physical_core);
    }

    for (auto &t : threads)
        t.join();
    return 0;
}