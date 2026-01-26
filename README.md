# ⚡ DbSync: Shared-Nothing Key-Value Engine

**DbSync** is a high-performance, sharded key-value store built in **C++20** using **Linux io_uring**. It leverages a **Shared-Nothing Architecture** to eliminate lock contention and cache thrashing, enabling a single process to outperform an 8-node Redis cluster.

## 📊 Phase-Wise Progress

| Phase | Architecture | Key Feature | Throughput (GET) | P99 Latency |
| --- | --- | --- | --- | --- |
| **Phase 1** | Synchronous | Protocol Implementation | 18 RPS | 0.03 ms |
| **Phase 2** | Thread Pool | Task Concurrency | 73 RPS | 0.12 ms |
| **Phase 3** | io_uring | Async I/O (Single Ring) | 200,000 RPS | 0.37 ms |
| **Phase 4** | Multi-Reactor | Horizontal Scaling | 265,000 RPS | 4.27 ms |
| **Phase 5** | Zero-Alloc | Object Pooling | 332,889 RPS | 0.93 ms |
| **Phase 6** | **Shared-Nothing** | **Lock-Free Local Engines** | **3,944,773 RPS** | **1.89 ms** |

---

## 🛠️ Performance Breakthroughs

### 1. Eliminating the "Synchronization Tax" (Phase 6)

* **The Problem:** Even with 4,096 shards, threads were fighting over a global array. This caused **False Sharing**, where writing to one shard invalidated the CPU caches of all other cores.
* **The Solution:** **Engine Fragmentation**. Each Reactor thread now owns its own `LocalEngine`. Data is physically bound to the CPU core that manages it, resulting in zero cross-core cache churn.

### 2. Lock-Free Sharding

* **The Problem:** Millions of atomic `lock` operations per second acted as memory barriers, stalling CPU instruction pipelines.
* **The Solution:** By ensuring only one thread ever accesses its specific engine instance, we **removed all mutexes**. The hot path is now 100% lock-free.

### 3. The io_uring Multi-Reactor

* **Async I/O:** Each reactor thread manages its own `io_uring` instance, handling thousands of connections without blocking.
* **Zero-Copy:** Uses `std::string_view` for RESP parsing and per-thread object pools to avoid the global heap allocator lock (`glibc malloc`).

---

## 🏗️ Phase 6 Architecture

* **Network:** Multi-threaded event loops via `SO_REUSEPORT`.
* **Isolation:** **Shared-Nothing**. No memory is shared between Reactor threads.
* **Hardware:** **Physical Core Affinity**. Threads are pinned to specific cores to maximize L1/L2 cache "warmth."
* **Storage:** Localized `absl::flat_hash_map` for O(1) lookups with zero locking overhead.

---

## 📂 Project Structure

```text
DbSync/
├── include/              # Header files
│   ├── dbsync_engine.hpp # Lock-Free Local Engine Logic
│   ├── resp_parser.hpp   # Zero-copy RESP Parser
│   └── network.hpp       # io_uring & UDS/TCP handling
├── src/                  # Implementation
│   ├── main.cpp          # Multi-Reactor Entry Point
│   └── ...
└── tests/                # Benchmarks (redis-benchmark)

```

---

## 🚀 Benchmarking

To achieve the peak **~4M RPS** throughput, run the benchmark against the Unix Domain Socket with high pipelining:

```bash
# 1. Compile with -O3 for maximum speed
g++ -std=c++20 -O3 main.cpp -o DbSync -luring -pthread

# 2. Run the server
./DbSync

# 3. Execute the Drag Race (8 Threads, 32 Pipeline Depth)
redis-benchmark -s /tmp/dbsync.sock -t set,get -n 2000000 -c 100 -P 32 --threads 8

```

---
