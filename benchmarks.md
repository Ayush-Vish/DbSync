
# ⚡ DbSync: High-Performance Key-Value Storage

**DbSync** is a sharded, lock-free key-value store built in **C++20** using **Linux io_uring**. It utilizes a shared-nothing architecture to fully saturate modern multi-core CPUs, achieving performance parity with distributed clusters on a single node.

## 📊 Phase-Wise Progress

| Phase | Architecture | Key Feature | Throughput (GET) | P99 Latency |
| --- | --- | --- | --- | --- |
| **Phase 1** | Synchronous | Protocol Implementation | 18 RPS | 0.03 ms |
| **Phase 2** | Thread Pool | Task Concurrency | 73 RPS | 0.12 ms |
| **Phase 3** | io_uring | Async I/O (Single Ring) | 200,000 RPS | 0.37 ms |
| **Phase 4** | Multi-Reactor | Horizontal Scaling | 265,000 RPS | 4.27 ms |
| **Phase 5** | Zero-Alloc | Hardware Optimization | 332,889 RPS | 0.93 ms |
| **Phase 6** | **Shared-Nothing** | **Lock-Free Local Engines** | **3,944,773 RPS** | **1.89 ms** |

---

## 🛠 Problems & Solutions

### 1. The Cache Invalidation "Tax" (Phase 5 → 6)

* **Problem:** Even with 4,096 shards, threads were fighting over a global array. When one core wrote to a shard, it forced a "Cache Invalidation" across all other cores, stalling the CPU memory bus.
* **Solution:** **Engine Fragmentation**. Each Reactor thread now owns its own `LocalEngine` instance. Data is physically isolated to the core it lives on, meaning zero cross-core cache churn.

### 2. Mutex/Atomic Contention (Phase 5 → 6)

* **Problem:** Millions of `shared_lock` and `unique_lock` operations per second created an "Atomic Barrier" that prevented the CPU from reordering instructions for maximum speed.
* **Solution:** **Lock-Free Sharding**. Because each thread exclusively owns its shards, we deleted all mutexes and locks. The engine now runs at raw memory-access speeds.

### 3. Ingress "Front-Door" Bottleneck

* **Problem:** Multiple threads calling `accept()` on a single TCP port or Unix Socket caused a kernel-level lock contention.
* **Solution:** **SO_REUSEPORT / Multi-Socket**. Distributing the connection load across multiple "entrances" allows the kernel to hand off new clients to reactors without serializing them.

---

## 🏗 Phase 6 Architecture (Current)

* **Architecture:** **Shared-Nothing**. No memory is shared between Reactor threads.
* **Memory:** Per-thread `absl::flat_hash_map` with zero-allocation connection pooling.
* **I/O:** Multi-ring `io_uring` setup with 1:1 Physical Core mapping.
* **Synchronization:** **Zero.** No mutexes, no atomics in the hot path.

---

## 🚀 How to Benchmark

To replicate the peak **3.9M+ RPS** performance, use the multi-threaded benchmark tool with heavy pipelining:

```bash
# Compile with maximum optimization
g++ -std=c++20 -O3 main.cpp -o DbSync -luring -pthread

# Run Server
./DbSync

# Run Benchmark (Matching thread count to physical cores)
redis-benchmark -s /tmp/dbsync.sock -t set,get -n 2000000 -c 100 -P 32 --threads 8

```

