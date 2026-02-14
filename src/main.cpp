#include <iostream>
#include <thread>
#include <unordered_map>
#include <liburing.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "../include/connection.hpp"
#include "../include/command_handler.hpp"
#include "../include/itc_queue.hpp"

// Global ITC context shared by all reactors
ITCContext g_itc;
int g_num_reactors = 1;

int create_shared_socket()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket failed");
        return -1;
    }
    
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt SO_REUSEADDR failed");
        close(fd);
        return -1;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt SO_REUSEPORT failed");
        close(fd);
        return -1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Bind failed");
        close(fd);
        return -1;
    }
    if (listen(fd, 1024) < 0)
    {
        perror("listen failed");
        close(fd);
        return -1;
    }
    return fd;
}

void run_reactor(int reactor_id, int physical_core_id)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(physical_core_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    // Shared-nothing: each reactor owns its partition of the global keyspace
    DbSyncEngine engine(512, reactor_id, g_num_reactors);

    ConnectionPool conn_pool(MAX_CONN_PER_THREAD);

    // Map fd -> Connection* for ITC response handling
    std::unordered_map<int, Connection*> fd_to_conn;

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
        Connection *conn = conn_pool.acquire();
        if (!conn)
            return;
        conn->fd = thread_socket;
        conn->type = OpType::ACCEPT;
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        io_uring_prep_accept(sqe, thread_socket, nullptr, nullptr, 0);
        io_uring_sqe_set_data(sqe, conn);
    };

    submit_accept();

    // Register eventfd with io_uring for ITC wakeup
    Connection* itc_conn = conn_pool.acquire();
    if (itc_conn) {
        itc_conn->fd = g_itc.get_event_fd(reactor_id);
        itc_conn->type = OpType::ITC_EVENT;
    }
    
    // Buffer for eventfd reads
    static thread_local uint64_t eventfd_buf;
    
    auto submit_itc_read = [&]() {
        if (itc_conn) {
            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
            io_uring_prep_read(sqe, itc_conn->fd, &eventfd_buf, sizeof(eventfd_buf), 0);
            io_uring_sqe_set_data(sqe, itc_conn);
        }
    };
    
    submit_itc_read();

    // Use resp::bulk from command_handler.hpp for formatting

    // Helper to submit response when connection is ready
    auto submit_response = [&](Connection* c) {
        if (c->pending_itc > 0) {
            c->suspended = true;
            return; // Wait for ITC responses
        }
        
        // Build response from pipeline results if we have them
        if (!c->pipeline_results.empty()) {
            c->response_data.clear();
            for (const auto& r : c->pipeline_results) {
                c->response_data += r;
            }
            c->pipeline_results.clear();
        }
        
        if (!c->response_data.empty()) {
            c->type = OpType::WRITE;
            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
            io_uring_prep_send(sqe, c->fd, c->response_data.c_str(), c->response_data.size(), 0);
            io_uring_sqe_set_data(sqe, c);
        } else {
            c->type = OpType::READ;
            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
            io_uring_prep_read(sqe, c->fd, c->buffer, 4096, 0);
            io_uring_sqe_set_data(sqe, c);
        }
    };

    while (true)
    {
        // ========== PHASE 1: Drain ITC Inbox ==========
        uint64_t signal_mask = 0;
        
        ITCMessage msg;
        while (g_itc.pop(reactor_id, msg)) {
            if (msg.type == ITCMessage::Type::GET_REQ) {
                // Process GET request from another reactor
                auto result = engine.get(msg.get_key());
                
                ITCMessage resp;
                resp.type = ITCMessage::Type::RESP;
                resp.sender_reactor = reactor_id;
                resp.conn_fd = msg.conn_fd;
                resp.pipeline_idx = msg.pipeline_idx;
                resp.found = result.has_value();
                if (result) resp.set_value(*result);
                
                g_itc.push(msg.sender_reactor, resp);
                signal_mask |= (1ULL << msg.sender_reactor);
            }
            else if (msg.type == ITCMessage::Type::SET_REQ) {
                // Process SET request from another reactor
                process_local_set(engine, msg.get_key(), msg.get_value(), msg.ttl_ms);
                
                // Write to AOF for remote SETs
                if (engine.aof_fd >= 0) {
                    char aof_buf[1024];
                    int aof_len = snprintf(aof_buf, sizeof(aof_buf),
                        "*3\r\n$3\r\nSET\r\n$%u\r\n%.*s\r\n$%u\r\n%.*s\r\n",
                        (unsigned)msg.key_len, (int)msg.key_len, msg.key,
                        (unsigned)msg.value_len, (int)msg.value_len, msg.value);
                    [[maybe_unused]] auto _ = write(engine.aof_fd, aof_buf, aof_len);
                }
                
                ITCMessage resp;
                resp.type = ITCMessage::Type::RESP;
                resp.sender_reactor = reactor_id;
                resp.conn_fd = msg.conn_fd;
                resp.pipeline_idx = msg.pipeline_idx;
                resp.found = true;
                // For SET, found=true means success, value not used
                
                g_itc.push(msg.sender_reactor, resp);
                signal_mask |= (1ULL << msg.sender_reactor);
            }
            else if (msg.type == ITCMessage::Type::RESP) {
                // Response came back for our request
                auto it = fd_to_conn.find(msg.conn_fd);
                if (it != fd_to_conn.end()) {
                    Connection* c = it->second;
                    
                    // Store result at correct pipeline position
                    if (msg.pipeline_idx < (int)c->pipeline_results.size()) {
                        // Check the original request type from the placeholder
                        // For SET response: +OK\r\n, for GET: bulk string or nil
                        if (msg.found) {
                            std::string_view val = msg.get_value();
                            if (val.empty()) {
                                c->pipeline_results[msg.pipeline_idx] = std::string(resp::OK);
                            } else {
                                c->pipeline_results[msg.pipeline_idx] = resp::bulk(val);
                            }
                        } else {
                            c->pipeline_results[msg.pipeline_idx] = std::string(resp::NIL);
                        }
                    }
                    
                    c->pending_itc--;
                    
                    // Check if all ITC responses are in
                    if (c->pending_itc == 0 && c->suspended) {
                        c->suspended = false;
                        submit_response(c);
                    }
                }
            }
        }
        
        // Batch signal all reactors we messaged
        for (int i = 0; i < g_num_reactors; i++) {
            if (signal_mask & (1ULL << i)) {
                g_itc.signal(i);
            }
        }

        // ========== PHASE 2: IO Processing ==========
        struct io_uring_cqe *cqe;
        io_uring_submit(&ring);
        if (io_uring_wait_cqe(&ring, &cqe) < 0)
            continue;

        Connection *conn = (Connection *)io_uring_cqe_get_data(cqe);
        int res = cqe->res;

        // Handle AOF_WRITE errors first (before generic res < 0 cleanup)
        if (conn->type == OpType::AOF_WRITE && res < 0)
        {
            // AOF write failed - log and cleanup connection
            fd_to_conn.erase(conn->fd);
            close(conn->fd);
            conn_pool.release(conn);
        }
        else if (res < 0)
        {
            if (conn->type != OpType::ACCEPT) {
                fd_to_conn.erase(conn->fd);
                close(conn->fd);
                conn_pool.release(conn);
            }
        }
        else if (conn->type == OpType::ACCEPT)
        {
            Connection *client = conn_pool.acquire();
            if (client)
            {
                client->fd = res;
                client->type = OpType::READ;
                client->owner_reactor = reactor_id;
                fd_to_conn[res] = client;  // Register for ITC lookups
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
                fd_to_conn.erase(conn->fd);
                close(conn->fd);
                conn_pool.release(conn);
            }
            else
            {
                // ========== PHASE 3: Pipelined Command Processing ==========
                std::string_view remaining(conn->buffer, res);
                conn->response_data.clear();
                conn->pipeline_results.clear();
                conn->pending_itc = 0;
                bool has_set_command = false;
                int cmd_idx = 0;
                uint64_t remote_signal_mask = 0;

                while (!remaining.empty())
                {
                    auto [cmd, consumed] = RespParser::parse(remaining);

                    if (consumed == 0)
                        break;

                    // Determine key ownership for GET/SET commands
                    std::string_view key;
                    if ((cmd.type == "GET" || cmd.type == "SET") && !cmd.args.empty()) {
                        key = cmd.args[0];
                    }

                    int owner = key.empty() ? reactor_id : engine.fast_owner(key);
                    bool is_local = (owner == reactor_id);

                    if (cmd.type == "SET" && cmd.args.size() >= 2)
                    {
                        uint64_t ttl_ms = parse_ttl(cmd);

                        if (is_local) {
                            engine.set(cmd.args[0], cmd.args[1], ttl_ms);
                            
                            // Prepare AOF
                            conn->aof_len = snprintf(
                                conn->aof_buf, sizeof(conn->aof_buf),
                                "*3\r\n$3\r\nSET\r\n$%zu\r\n%.*s\r\n$%zu\r\n%.*s\r\n",
                                cmd.args[0].size(), (int)cmd.args[0].size(), cmd.args[0].data(),
                                cmd.args[1].size(), (int)cmd.args[1].size(), cmd.args[1].data());
                            
                            has_set_command = true;
                            conn->pipeline_results.push_back(std::string(resp::OK));
                        } else {
                            // Forward SET to owner reactor via ITC
                            ITCMessage itc_msg;
                            itc_msg.type = ITCMessage::Type::SET_REQ;
                            itc_msg.sender_reactor = reactor_id;
                            itc_msg.conn_fd = conn->fd;
                            itc_msg.pipeline_idx = cmd_idx;
                            itc_msg.set_key(cmd.args[0]);
                            itc_msg.set_value(cmd.args[1]);
                            itc_msg.ttl_ms = ttl_ms;
                            
                            g_itc.push(owner, itc_msg);
                            remote_signal_mask |= (1ULL << owner);
                            
                            conn->pending_itc++;
                            conn->pipeline_results.push_back("");  // Placeholder
                        }
                    }
                    else if (cmd.type == "GET" && !cmd.args.empty())
                    {
                        if (is_local) {
                            auto val = engine.get(cmd.args[0]);
                            if (val)
                            {
                                conn->pipeline_results.push_back(resp::bulk(*val));
                            }
                            else
                            {
                                conn->pipeline_results.push_back(std::string(resp::NIL));
                            }
                        } else {
                            // Forward GET to owner reactor via ITC
                            ITCMessage itc_msg;
                            itc_msg.type = ITCMessage::Type::GET_REQ;
                            itc_msg.sender_reactor = reactor_id;
                            itc_msg.conn_fd = conn->fd;
                            itc_msg.pipeline_idx = cmd_idx;
                            itc_msg.set_key(cmd.args[0]);
                            
                            g_itc.push(owner, itc_msg);
                            remote_signal_mask |= (1ULL << owner);
                            
                            conn->pending_itc++;
                            conn->pipeline_results.push_back("");  // Placeholder
                        }
                    }
                    else if (cmd.type == "CONFIG")
                    {
                        conn->pipeline_results.push_back(std::string(resp::EMPTY_ARRAY));
                    }
                    else if (cmd.type == "EXPIRE" && cmd.args.size() >= 2)
                    {
                        uint64_t ttl_ms = 0;
                        auto arg = cmd.args[1];
                        std::from_chars(arg.data(), arg.data() + arg.size(), ttl_ms);
                        conn->pipeline_results.push_back(":1\r\n");
                    }
                    else if (cmd.type == "PING")
                    {
                        conn->pipeline_results.push_back(std::string(resp::PONG));
                    }
                    else
                    {
                        conn->pipeline_results.push_back(std::string(resp::ERR_UNKNOWN));
                    }

                    remaining.remove_prefix(consumed);
                    cmd_idx++;
                }

                conn->expected_results = cmd_idx;

                // Batch signal remote reactors
                for (int i = 0; i < g_num_reactors; i++) {
                    if (remote_signal_mask & (1ULL << i)) {
                        g_itc.signal(i);
                    }
                }

                if (has_set_command && conn->aof_len > 0 && conn->pending_itc == 0)
                {
                    // Write to AOF first (only if no pending ITC)
                    conn->type = OpType::AOF_WRITE;
                    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
                    io_uring_prep_write(sqe, engine.aof_fd, conn->aof_buf, conn->aof_len, -1);
                    sqe->flags |= IOSQE_ASYNC;
                    io_uring_sqe_set_data(sqe, conn);
                }
                else
                {
                    submit_response(conn);
                }
            }
        }
        else if (conn->type == OpType::AOF_WRITE)
        {
            conn->aof_len = 0;
            
            // Now send response to client
            submit_response(conn);
        }
        else if (conn->type == OpType::WRITE)
        {
            conn->type = OpType::READ;
            conn->response_data.clear();
            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
            io_uring_prep_read(sqe, conn->fd, conn->buffer, 4096, 0);
            io_uring_sqe_set_data(sqe, conn);
        }
        else if (conn->type == OpType::ITC_EVENT)
        {
            // ITC eventfd triggered - messages already processed in Phase 1
            // Re-submit the eventfd read for next wakeup
            submit_itc_read();
        }

        io_uring_cqe_seen(&ring, cqe);
    }
}

int main()
{
    std::cout << "🧹 Janitor active: Sampling shards for expired keys..." << std::endl;
    int logical_cores = std::thread::hardware_concurrency();
    int num_reactors = logical_cores / 2;
    if (num_reactors == 0)
        num_reactors = 1;

    // Initialize global ITC context for shared-nothing architecture
    g_num_reactors = num_reactors;
    g_itc.init(num_reactors);

    std::vector<std::thread> threads;
    std::cout << "🔥 DbSync Phase 6: Shared-Nothing Dragonfly Architecture" << std::endl;
    std::cout << "🚀 Core Pinning: Using " << num_reactors << " Physical Cores" << std::endl;
    std::cout << "📡 ITC: moodycamel ConcurrentQueue (" << num_reactors << " inboxes)" << std::endl;
    
    for (int i = 0; i < num_reactors; ++i)
    {
        int physical_core = i * 2;
        threads.emplace_back(run_reactor, i, physical_core);
    }

    for (auto &t : threads)
        t.join();
    return 0;
}
