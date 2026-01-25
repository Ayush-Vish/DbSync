# 📊 Redis vs DbSync Performance Comparison

## SET Operations

| Pipeline | Redis RPS | DbSync RPS | Speedup | Redis p99 | DbSync p99 |
|----------|-----------|------------|---------|-----------|------------|
| P=1 | 166,334 | 307,031 | **1.85x** | ms | ms |
| P=16 | 1,328,021 | 1,328,021 | **1.00x** | ms | ms |
| P=32 | 1,329,787 | 1,326,260 | **1.00x** | ms | ms |
| P=8 | 798,085 | 1,328,021 | **1.66x** | ms | ms |

## GET Operations

| Pipeline | Redis RPS | DbSync RPS | Speedup | Redis p99 | DbSync p99 |
|----------|-----------|------------|---------|-----------|------------|
| P=1 | 181,554 | 330,469 | **1.82x** | ms | ms |
| P=16 | 1,329,787 | 3,952,569 | **2.97x** | ms | ms |
| P=32 | 1,992,032 | 3,952,569 | **1.98x** | ms | ms |
| P=8 | 997,009 | 1,992,032 | **2.00x** | ms | ms |

## Summary

- **Average SET speedup**: 1.38x
- **Average GET speedup**: 2.19x

### Test Parameters
- **N**: 1,000,000 requests
- **C**: 100 parallel connections
- **P**: Pipeline size (1, 8, 16, 32)
- **Threads**: 8
- **p99**: 99th percentile latency
