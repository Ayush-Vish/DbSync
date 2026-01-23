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
#include <thread>
#include <queue>
#include <condition_variable>
#include <functional>
#include <liburing.h>

#include "../include/dbsync_engine.hpp"
#include "../include/resp_parser.hpp"
#include "../include/debug.hpp"
const int PORT = 6379;
const int QUEUE_DEPTH = 4096; // size of the ring for kernel requests

enum class OpType{
    ACCEPT, READ, WRITE
};

struct Connection {
    int fd;
    OpType type;
    char buffer[4096]; // data lands here directly from the kernel
    std::string response_data; // keeps the string alive while the kernel is sending it
};

void submit_read(struct io_uring *ring, Connection *conn) {
    conn->type = OpType::READ;
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_read(sqe, conn->fd, conn->buffer, sizeof(conn->buffer), 0);
    io_uring_sqe_set_data(sqe, conn); // tag this request so we find it in completion
}

void submit_accept(struct io_uring *ring, int server_fd) {
    Connection *conn = new Connection();
    conn->fd = server_fd;
    conn->type = OpType::ACCEPT;
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_accept(sqe, server_fd, nullptr, nullptr, 0);
    io_uring_sqe_set_data(sqe, conn);
}

void submit_write(struct io_uring *ring, Connection *conn, std::string resp) {
    conn->type = OpType::WRITE;
    conn->response_data = std::move(resp); // move string into the struct to keep memory alive
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_send(sqe, conn->fd, conn->response_data.c_str(), conn->response_data.size(), 0);
    io_uring_sqe_set_data(sqe, conn);
}

class ThreadPool {
private:
std::vector<std::thread> workers; // the actual threads waiting for work
    std::queue<std::function<void()>> tasks; // a queue of clients waiting to be handled
    std::mutex queue_mtx; // to make sure two threads don't grab the same client
    std::condition_variable cv; // to wake up threads when a new client arrives
    bool stop = false;

public:
    ThreadPool(size_t num_threads) {
        for(size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this]{
                // worker thread function
                while(true) {
                    std::function<void()> task ;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mtx);
                        cv.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }


            });
        }
    }
    void enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queue_mtx);
            tasks.push(std::move(task));
        }
        cv.notify_one(); // wake up one idle worker 
    }
    ~ThreadPool() {
        { std::unique_lock<std::mutex> lock(queue_mtx); stop = true; }
        cv.notify_all();
        for (auto &w : workers) w.join();
    }

};

void handle_client(int client_fd, DbSyncEngine &engine) {
    while(true) {

        char buffer[1024] = {0};
        ssize_t bytes_read = read(client_fd, buffer, 1024);
        if(bytes_read <= 0) {
            close(client_fd);
            break;
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
int main() {
    DbSyncEngine engine(32); // 32 shards to avoid any thread bottlenecks
    struct io_uring ring;
    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
        std::cerr << "Failed to init io_uring" << std::endl;
        return 1;
    }

    // Standard Socket Setup
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    sockaddr_in addr{AF_INET, htons(PORT), INADDR_ANY};
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    listen(server_fd, 1024);

    // Bootstrap the loop by asking for the first connection
    submit_accept(&ring, server_fd);

    std::cout << "🚀 DbSync Phase 3 (io_uring) live on port " << PORT << "..." << std::endl;

    while (true) {
        struct io_uring_cqe *cqe;
        io_uring_submit(&ring); // Push all queued requests to the kernel at once
        
        // Wait for at least one completion (batching happens here)
        if (io_uring_wait_cqe(&ring, &cqe) < 0) continue;

        Connection *conn = (Connection *)io_uring_cqe_get_data(cqe);
        int res = cqe->res; // result of the syscall (bytes read/new fd)

        if (res < 0) {
            if (conn->type != OpType::ACCEPT) { close(conn->fd); delete conn; }
        } else if (conn->type == OpType::ACCEPT) {
            int client_fd = res;
            Connection *client_conn = new Connection();
            client_conn->fd = client_fd;
            submit_read(&ring, client_conn); // Start reading from the new client
            std::cout << "Reading from client fd: " << client_fd << std::endl;
            submit_accept(&ring, server_fd); // Listen for the next client
            delete conn; // Cleanup original accept task
        } else if (conn->type == OpType::READ) {
            if (res == 0) { // Client closed connection
                close(conn->fd);
                delete conn;
            } else {
                TRACE_EVENT("Request_Pipeline_Total");
                std::string_view raw(conn->buffer, res);
                Command cmd;
                {
                    TRACE_EVENT("Request_Parsing");
                    cmd = RespParser::parse(raw);
                }

                std::string response;
                {
                    

                    if (cmd.type == "SET" && cmd.args.size() >= 2) {
                        engine.set(cmd.args[0], cmd.args[1]);
                        response = "+OK\r\n";
                    } else if (cmd.type == "GET" && !cmd.args.empty()) {
                        auto val = engine.get(cmd.args[0]);
                        if (val) {
                            std::string_view sv = *val;
                            response = std::string("$") + std::to_string(sv.size()) + "\r\n";
                            response.append(sv);
                            response.append("\r\n");
                        } else {
                            response = "$-1\r\n";
                        }
                    } else if (cmd.type == "PING") {
                        response = "+PONG\r\n";
                    } else {
                        response = "-ERR unknown command\r\n";
                    }
                    
                }
                // Simple Command Dispatcher
                submit_write(&ring, conn, std::move(response));
            }
        } else if (conn->type == OpType::WRITE) {
            // Write finished, immediately wait for the next command on this socket
            submit_read(&ring, conn);
        }

        io_uring_cqe_seen(&ring, cqe); // Tell kernel we processed this completion
    }

    io_uring_queue_exit(&ring);
    return 0;
}
