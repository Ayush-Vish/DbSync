#include <liburing.h>
#include <vector>
#include <string_view>

// We need to track what kind of operation finished
enum class OpType { ACCEPT, READ, WRITE };

struct Connection {
    int fd;
    OpType type;
    char buffer[4096]; // Each connection gets its own buffer
};

class IoContext {
private:
    struct io_uring ring;
    const int QUEUE_DEPTH = 4096; // How many requests the kernel can hold

public:
    IoContext() {
        io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
    }

    // Tell the kernel: "Tell me when someone connects"
    void prepare_accept(int server_fd, Connection* conn) {
        conn->type = OpType::ACCEPT;
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        io_uring_prep_accept(sqe, server_fd, nullptr, nullptr, 0);
        io_uring_sqe_set_data(sqe, conn); // Attach our object so we know who this is later
    }

    // Tell the kernel: "Tell me when this client sends data"
    void prepare_read(Connection* conn) {
        conn->type = OpType::READ;
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        io_uring_prep_read(sqe, conn->fd, conn->buffer, sizeof(conn->buffer), 0);
        io_uring_sqe_set_data(sqe, conn);
    }

    // Tell the kernel: "Tell me when you've finished sending this response"
    void prepare_write(Connection* conn, std::string_view response) {
        conn->type = OpType::WRITE;
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
        // Warning: response must stay alive until completion!
        io_uring_prep_send(sqe, conn->fd, response.data(), response.size(), 0);
        io_uring_sqe_set_data(sqe, conn);
    }

    void submit() {
        io_uring_submit(&ring);
    }

    // This is the heart: it waits for the kernel to finish something
    int wait_for_completion(struct io_uring_cqe **cqe) {
        return io_uring_wait_cqe(&ring, cqe);
    }

    void mark_seen(struct io_uring_cqe *cqe) {
        io_uring_cqe_seen(&ring, cqe);
    }
};
