# ⚡ DbSync: Shared-Nothing Key-Value Engine

**DbSync** is an ultra-high-performance, sharded key-value store built in **C++20** using **Linux io_uring**. By transitioning to a **Shared-Nothing Architecture**, it eliminates the performance ceilings of traditional mutex-based systems, enabling linear vertical scaling that outperforms even highly optimized clusters like Redis and DragonflyDB.

---

## 📊 Competitive Benchmarks

The following data represents a head-to-head comparison using `memtier_benchmark` (8 threads, 200 clients, 1:1 SET/GET ratio). DbSync consistently demonstrates superior throughput and significantly lower tail latency (P99).

| Engine | Total Throughput (Ops/sec) | Avg. Latency (ms) | P50 Latency (ms) | P99 Latency (ms) |
| --- | --- | --- | --- | --- |
| **DbSync (Async AOF)** | **2,140,143** | **11.95** | **10.56** | **34.05** |
| **Redis** | 1,225,170 | 20.89 | 19.58 | 41.47 |
| **DragonflyDB** | 1,161,721 | 22.02 | 18.30 | 75.78 |

> **Analysis:** DbSync outperforms Redis by **~75%** and DragonflyDB by **~84%** in total throughput. Notably, DbSync's **P99 latency is ~2.2x lower than Dragonfly's**, proving the efficiency of the lock-free steering model combined with batched, non-blocking AOF persistence.

---


## 📊 Evolution of Throughput

> **Note on Benchmarking:** Up to Phase 6, `redis-benchmark` was used for local validation. However, as the engine matured into a distributed steering model, we migrated to `memtier_benchmark`. Unlike the former, `memtier` provides a more rigorous multi-threaded load profile, revealing the true performance of the Inter-Thread Communication (ITC) bus under sustained pressure.

| Phase | Architecture | Key Feature | Throughput | P99 Latency |
| --- | --- | --- | --- | --- |
| **Phase 1** | Synchronous | Basic RESP Protocol | 18 RPS | 0.03 ms |
| **Phase 2** | Thread Pool | Task Concurrency | 73 RPS | 0.12 ms |
| **Phase 3** | io_uring | Async I/O (Single Ring) | 200,000 RPS | 0.37 ms |
| **Phase 4** | Multi-Reactor | Horizontal Scaling | 265,000 RPS | 4.27 ms |
| **Phase 5** | Zero-Alloc | Object/Connection Pooling | 332,889 RPS | 0.93 ms |
| **Phase 6** | Shared-Nothing | Lock-Free Local Engines | 3,944,773 RPS* | 1.89 ms |
| **Phase 7** | Distributed Steering | SPSC Request Hopping | 365,825 RPS | 0.79 ms |
| **Phase 8** | **Async Batched AOF** | **Non-Blocking io_uring Persistence** | **2,140,143 Ops/sec** | **34.05 ms** |

**Phase 6 throughput reflects independent local shards without cross-core consistency logic.*

---

## 🏗️ Phase-Wise Architectural Journey

### Phase 1 - 3: The Foundation

* **Protocol:** Implemented a zero-copy **RESP (Redis Serialization Protocol)** parser using `std::string_view` to minimize heap allocations.
* **Async Core:** Transitioned from blocking sockets to **Linux io_uring**, utilizing submission/completion queues to eliminate thread-per-connection overhead.

### Phase 4 - 5: Scaling & Optimization

* **Multi-Reactor:** Launched independent event loops on multiple threads using `SO_REUSEPORT`.
* **Zero-Allocation:** Built a per-thread **Object Pool** for connection structures, bypassing the `malloc` global lock and stabilizing P99 latencies.

### Phase 6: The Shared-Nothing Breakthrough

* **The Problem:** Cross-thread shard locking caused **False Sharing**, where writing to one core invalidated the CPU caches of others.
* **The Solution:** Fragmented the engine so each Reactor thread owns its private data partition. This removed all mutexes from the hot path.

### Phase 7: Distributed Steering

* **Consistency Logic:** Implemented an **ITC (Inter-Thread Communication) Bus** using SPSC (Single-Producer Single-Consumer) queues to "steer" requests.
* **The "Hop":** If Reactor A accepts a connection for a key owned by Reactor B, it asynchronously "hops" the request to Core B's inbox via `eventfd` signaling.

### Phase 8: Async Batched AOF (Current)

* **The Problem:** The previous AOF gated every `SET` response on its own synchronous write completing, and remote (hopped) SETs used a blocking `write()` syscall on the reactor hot path — capping pipelined SET throughput at ~1M ops/sec.
* **The Solution:** Each reactor coalesces all SETs into a per-core in-memory buffer and flushes it as **one batched, fire-and-forget `io_uring` write per event-loop iteration** (double-buffered for write-lifetime safety). Client responses are no longer gated on persistence.
* **The Result:** Pipelined SET throughput roughly **doubled (≈1M → ≈2M ops/sec)** and the mixed 1:1 workload reached **2.14M ops/sec**, at the cost of a small async-durability window (a crash can lose the most recent, not-yet-flushed writes — the Redis `appendfsync no` tradeoff).

---

## 🛠️ Key Technical Breakthroughs

### 1. SPSC Inter-Thread Bus

DbSync utilizes a high-speed matrix of SPSC queues (one for every core-to-core path). This allows reactors to communicate without the atomic `compare-and-swap` overhead required by MPMC queues.

### 2. Physical Core Affinity

Threads are pinned to **Physical Cores** skipping hyper-threaded siblings. This ensures each reactor has exclusive access to its L1/L2 cache, maximizing "cache warmth."

### 3. Generation-Based Tracking

To prevent race conditions during asynchronous hopping, each connection carries a `generation` ID. This ensures that stale ITC responses are never sent to a recycled file descriptor.

---

## 🚀 Benchmarking (Phase 8)

Verified via `memtier_benchmark` with 8 threads and 200 concurrent connections:

```bash
# Compile with maximum optimization
g++ -std=c++20 -O3 main.cpp -o DbSync -luring -pthread

# Execute High-Pressure Consistency Test
memtier_benchmark -s 127.0.0.1 -p 6379 --protocol=redis --clients=200 --threads=8 --ratio=1:1 --test-time=60

```

**Verified Results:**

* **Total Throughput:** 2,140,143 Ops/sec
* **Avg Latency:** 11.95 ms
* **p99 Latency:** 34.05 ms
* **Consistency:** 100% (Zero architectural misses across all 8 cores)

---

## 📂 Project Structure

```text
DbSync/
├── include/              
│   ├── dbsync_engine.hpp # Sharded, Lock-Free Storage
│   ├── itc_queue.hpp     # SPSC Bus & eventfd Signaling
│   └── resp_parser.hpp   # Zero-copy RESP Parser
├── src/                  
│   └── main.cpp          # Multi-Reactor & Steering Logic
└── appendonly_N.aof      # Per-core Write-Ahead Logs

```

