
# ⚡ DbSync: High-Performance Key-Value Storage

**DbSync** is a sharded, asynchronous key-value store built in **C++20** using **Linux io_uring**. It is designed to demonstrate how hardware-level optimizations and modern kernel primitives can push a single node beyond **330,000 requests per second**.

## 📊 Phase-Wise Progress

| Phase | Architecture | Key Feature | Throughput (GET) | P99 Latency |
| --- | --- | --- | --- | --- |
| **Phase 1** | Synchronous | Protocol Implementation | 18 RPS | 0.03 ms |
| **Phase 2** | Thread Pool | Task Concurrency | 73 RPS | 0.12 ms |
| **Phase 3** | io_uring | Async I/O (Single Ring) | 200,000 RPS | 0.37 ms |
| **Phase 4** | Multi-Reactor | Horizontal Scaling | 265,000 RPS | 4.27 ms |
| **Phase 5** | **Zero-Alloc** | **Hardware Optimization** | **332,889 RPS** | **0.93 ms** |

---

## 🛠 Problems & Solutions

### 1. The Syscall Bottleneck (Phase 1 → 3)

* **Problem:** Every `read` and `write` was a system call, causing constant context switching between user space and kernel space.
* **Solution:** Implemented **`io_uring`**. By using a shared ring buffer between the kernel and user space, we batched multiple I/O operations into a single submission, reducing context switches by over 90%.

### 2. Lock Contention & Queue Blocking (Phase 2)

* **Problem:** A centralized `ThreadPool` queue required a global lock. As thread counts increased, cores spent more time "waiting in line" for the lock than doing work.
* **Solution:** Moved to a **Shared-Nothing Multi-Reactor** architecture. Each core owns its own independent `io_uring` ring and local task management.

### 3. The "Silent Killer": Global Heap Allocator (Phase 4 → 5)

* **Problem:** Using `new` and `delete` in a high-frequency loop (200k+ times per second) triggered the global heap lock in `glibc`, causing massive P99 latency spikes (4ms+).
* **Solution:** Implemented a **Per-Thread Object Pool**. Connection objects are pre-allocated and reused, resulting in **zero heap allocations** in the hot path.

### 4. Cache Thrashing (Phase 4 → 5)

* **Problem:** The OS scheduler was moving threads between logical cores, flushing the L1/L2 caches and slowing down memory access.
* **Solution:** **Physical Core Affinity**. Threads are pinned to physical CPU cores, ensuring "cache warmth" and avoiding resource sharing with Hyper-Thread siblings.

---

## 🏗 Final Phase 5 Architecture

* **Network:** Multi-Reactor pattern via `SO_REUSEPORT`.
* **Memory:** Lock-free, per-thread connection pooling.
* **Storage:** High-density sharded map (2048 shards) to eliminate mutex collisions.
* **Parsing:** Zero-copy RESP parsing via `std::string_view`.

---

## 🚀 How to Benchmark

Compile with high optimization and run the multi-threaded benchmark tool:

```bash
# Compile
g++ -std=c++20 -O3 main.cpp -o DbSync -luring -pthread

# Run Server
./DbSync

# Benchmark (from 8 client threads)
redis-benchmark -p 6379 -t set,get -n 1000000 -c 100 --threads 8

```


