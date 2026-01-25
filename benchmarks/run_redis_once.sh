#!/usr/bin/env bash
set -e

PORT=6379
OUT="redis_baseline.csv"
REDIS_BIN="redis-server"

NS=(1000000)
CS=(100)
PS=(1 8 16 32)
THREADS=(8)
TESTS=("set" "get")

echo "Running Redis baseline (one time only)..."
echo "test,n,c,p,threads,throughput_rps,avg_ms,p50_ms,p95_ms,p99_ms,max_ms" > "$OUT"

kill_port() {
  fuser -k ${PORT}/tcp 2>/dev/null || true
  sleep 1
}

start_redis() {
  kill_port
  $REDIS_BIN --save "" --appendonly no --port $PORT > /tmp/redis.log 2>&1 &
  REDIS_PID=$!
  sleep 2
  echo "✅ Redis started (PID: $REDIS_PID)"
}

stop_redis() {
  kill $REDIS_PID 2>/dev/null || true
  sleep 1
  kill_port
  echo "✅ Redis stopped"
}

run_one() {
  local TEST=$1
  local N=$2
  local C=$3
  local P=$4
  local T=$5

  echo "[REDIS] Running: $TEST N=$N C=$C P=$P T=$T"
  
  CMD_DETAILED="redis-benchmark -p $PORT -t $TEST -n $N -c $C -P $P --threads $T"
  DETAILED=$($CMD_DETAILED 2>&1)
  
  # Extract throughput
  THR=$(echo "$DETAILED" | grep "throughput summary:" | awk '{print $3}')
  
  # Extract latency summary - the line after "latency summary (msec):"
  # It looks like: "        avg       min       p50       p95       p99       max"
  #                "      0.190     0.008     0.079     0.647     0.895    28.575"
  LAT=$(echo "$DETAILED" | grep -A2 "latency summary (msec):" | tail -n1 | tr -s ' ')
  
  # Parse the space-separated values
  AVG=$(echo "$LAT" | awk '{print $1}')
  MIN=$(echo "$LAT" | awk '{print $2}')
  P50=$(echo "$LAT" | awk '{print $3}')
  P95=$(echo "$LAT" | awk '{print $4}')
  P99=$(echo "$LAT" | awk '{print $5}')
  MAX=$(echo "$LAT" | awk '{print $6}')

  echo "  → RPS: $THR, p99: ${P99}ms"
  echo "$TEST,$N,$C,$P,$T,$THR,$AVG,$P50,$P95,$P99,$MAX" >> "$OUT"
}

start_redis
for TEST in "${TESTS[@]}"; do
  for N in "${NS[@]}"; do
    for C in "${CS[@]}"; do
      for P in "${PS[@]}"; do
        for T in "${THREADS[@]}"; do
          run_one "$TEST" "$N" "$C" "$P" "$T"
        done
      done
    done
  done
done
stop_redis

echo ""
echo "✅ Redis baseline saved to $OUT"
echo "📝 You only need to run this once!"
