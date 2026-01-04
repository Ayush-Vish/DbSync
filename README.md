# DbSync ⚡

**DbSync** is a high-performance, sharded, key-value storage engine built in C++20. It aims to achieve ultra-low latency and high throughput by leveraging modern hardware primitives, asynchronous I/O, and lock-free data structures.

## 🎯 Project Goals

* **Faster than Redis:** Outperform Redis in multi-core environments by eliminating the single-threaded bottleneck.
* **Low Latency:** Achieve sub-millisecond P99 response times.
* **Modern I/O:** Utilize Linux `io_uring` for true asynchronous, zero-copy networking.
* **Scalability:** Use keyspace sharding to scale linearly with CPU cores.

---

## 🏗️ Architecture & Flow

The lifecycle of a request in **DbSync** follows a highly optimized pipeline:

1. **Network Ingress:** Multi-threaded event loop handles incoming TCP connections using `SO_REUSEPORT`.
2. **Request Parsing:** Raw RESP (Redis Serialization Protocol) bytes are parsed using `std::string_view` to avoid heap allocations and string copies.
3. **Command Dispatch:** The key is hashed to determine its **Shard**.
4. **Storage Engine:** The operation is performed on a thread-safe, sharded hash map.
5. **Network Egress:** Responses are sent back using asynchronous vectored writes.

---

## 🛠️ Roadmap: From Simple to "Faster than Redis"

### Phase 1: The Foundation (Current)

* **Networking:** Basic synchronous TCP sockets (Berkeley Sockets).
* **Storage:** `std::unordered_map` sharded with `std::mutex`.
* **Protocol:** Basic RESP support (`SET`, `GET`, `DEL`).
* **Goal:** Verify correctness and protocol compatibility with `redis-cli`.

### Phase 2: Concurrent Scaling

* **Optimization:** Implement **Read-Write Locks** (`std::shared_mutex`) to allow multiple simultaneous readers per shard.
* **Concurrency:** Move to a Multi-Reactor pattern where each thread owns its own event loop.

### Phase 3: The "Fast" Path (io_uring)

* **Optimization:** Replace the standard socket API with **`io_uring`**.
* **Zero-Copy:** Use **Fixed Buffers** and **Registered Files** to bypass the kernel-user space memory copying overhead.
* **SQPOLL:** Enable Submission Queue Polling to minimize system call overhead.

### Phase 4: Hardware Optimization

* **Data Structure:** Swap `std::unordered_map` for a **Flat Hash Map** (Open Addressing) for better CPU cache locality.
* **Memory:** Implement a **Slab Allocator** to manage memory pools and prevent fragmentation.
* **Affinity:** Pin threads to specific CPU cores to minimize cache misses.

---

## 📂 Project Structure

```text
DbSync/
├── include/              # Header files
│   ├── engine.hpp        # Sharded Storage Logic
│   ├── protocol.hpp      # Zero-copy RESP Parser
│   └── network.hpp       # io_uring & Socket handling
├── src/                  # Implementation
│   ├── main.cpp          # Entry Point
│   └── ...
├── tests/                # Benchmarks & Unit Tests
└── CMakeLists.txt        # Build System

```

---

## 🚀 Building the Project

Ensure you have a C++20 compliant compiler and `liburing` installed.

```bash
# Clone the repository
git clone https://github.com/yourusername/DbSync.git
cd DbSync

# Build the project
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# Run the server
./DbSync

```

---

## 🧪 Testing with Redis-CLI

Since **DbSync** speaks RESP, you can use the standard Redis toolset:

```bash
redis-cli -p 6379 SET mykey "Hello DbSync"
redis-cli -p 6379 GET mykey

```
