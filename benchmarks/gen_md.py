import csv
import os

# Check if required files exist
if not os.path.exists("redis_baseline.csv"):
    print("❌ redis_baseline.csv not found!")
    print("   Run './run_redis_once.sh' first to create baseline")
    exit(1)

if not os.path.exists("results.csv"):
    print("❌ results.csv not found!")
    print("   Run './run_dbsync.sh' first to benchmark DbSync")
    exit(1)

# Load Redis baseline
redis = {}
with open("redis_baseline.csv") as f:
    reader = csv.DictReader(f)
    for r in reader:
        key = (r["test"], r["n"], r["c"], r["p"], r["threads"])
        redis[key] = r

# Load DbSync results
db = {}
with open("results.csv") as f:
    reader = csv.DictReader(f)
    for r in reader:
        key = (r["test"], r["n"], r["c"], r["p"], r["threads"])
        db[key] = r

# Generate markdown report
with open("RESULTS.md", "w") as out:
    out.write("# 📊 Redis vs DbSync Performance Comparison\n\n")
    
    # SET Performance
    out.write("## SET Operations\n\n")
    out.write("| Pipeline | Redis RPS | DbSync RPS | Speedup | Redis p99 | DbSync p99 |\n")
    out.write("|----------|-----------|------------|---------|-----------|------------|\n")

    set_speedups = []
    for k in sorted(redis.keys()):
        if k[0] != "set":
            continue
        r = redis[k]
        d = db.get(k)
        if not d:
            continue

        speedup = float(d["throughput_rps"]) / float(r["throughput_rps"])
        set_speedups.append(speedup)

        out.write(
            f"| P={r['p']} | {float(r['throughput_rps']):,.0f} | "
            f"{float(d['throughput_rps']):,.0f} | "
            f"**{speedup:.2f}x** | {r['p99_ms']}ms | {d['p99_ms']}ms |\n"
        )

    # GET Performance
    out.write("\n## GET Operations\n\n")
    out.write("| Pipeline | Redis RPS | DbSync RPS | Speedup | Redis p99 | DbSync p99 |\n")
    out.write("|----------|-----------|------------|---------|-----------|------------|\n")

    get_speedups = []
    for k in sorted(redis.keys()):
        if k[0] != "get":
            continue
        r = redis[k]
        d = db.get(k)
        if not d:
            continue

        speedup = float(d["throughput_rps"]) / float(r["throughput_rps"])
        get_speedups.append(speedup)

        out.write(
            f"| P={r['p']} | {float(r['throughput_rps']):,.0f} | "
            f"{float(d['throughput_rps']):,.0f} | "
            f"**{speedup:.2f}x** | {r['p99_ms']}ms | {d['p99_ms']}ms |\n"
        )

    # Summary
    out.write("\n## Summary\n\n")
    if set_speedups:
        avg_set = sum(set_speedups) / len(set_speedups)
        out.write(f"- **Average SET speedup**: {avg_set:.2f}x\n")
    if get_speedups:
        avg_get = sum(get_speedups) / len(get_speedups)
        out.write(f"- **Average GET speedup**: {avg_get:.2f}x\n")
    
    out.write(f"\n### Test Parameters\n")
    out.write(f"- **N**: 1,000,000 requests\n")
    out.write(f"- **C**: 100 parallel connections\n")
    out.write(f"- **P**: Pipeline size (1, 8, 16, 32)\n")
    out.write(f"- **Threads**: 8\n")
    out.write(f"- **p99**: 99th percentile latency\n")

print("✅ RESULTS.md updated successfully!")
if set_speedups or get_speedups:
    print(f"\n📈 Performance Summary:")
    if set_speedups:
        print(f"   SET: {sum(set_speedups)/len(set_speedups):.2f}x faster (avg)")
    if get_speedups:
        print(f"   GET: {sum(get_speedups)/len(get_speedups):.2f}x faster (avg)")
