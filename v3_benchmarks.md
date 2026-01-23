``` 

{18:34}~/Desktop/DB:master ✗ ➭ redis-benchmark -p 6379 -t set,get -n 10000 -c 50
WARNING: Could not fetch server CONFIG
====== SET ======                                         
  10000 requests completed in 0.05 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.039 milliseconds (cumulative count 1)
50.000% <= 0.127 milliseconds (cumulative count 6212)
75.000% <= 0.135 milliseconds (cumulative count 7670)
87.500% <= 0.151 milliseconds (cumulative count 8765)
93.750% <= 0.223 milliseconds (cumulative count 9406)
96.875% <= 0.295 milliseconds (cumulative count 9688)
98.438% <= 0.359 milliseconds (cumulative count 9865)
99.219% <= 0.391 milliseconds (cumulative count 9925)
99.609% <= 0.791 milliseconds (cumulative count 9961)
99.805% <= 0.935 milliseconds (cumulative count 9982)
99.902% <= 1.007 milliseconds (cumulative count 9991)
99.951% <= 1.039 milliseconds (cumulative count 9996)
99.976% <= 1.047 milliseconds (cumulative count 9999)
99.994% <= 1.063 milliseconds (cumulative count 10000)
100.000% <= 1.063 milliseconds (cumulative count 10000)

Cumulative distribution of latencies:
0.180% <= 0.103 milliseconds (cumulative count 18)
93.270% <= 0.207 milliseconds (cumulative count 9327)
97.270% <= 0.303 milliseconds (cumulative count 9727)
99.410% <= 0.407 milliseconds (cumulative count 9941)
99.500% <= 0.503 milliseconds (cumulative count 9950)
99.630% <= 0.807 milliseconds (cumulative count 9963)
99.770% <= 0.903 milliseconds (cumulative count 9977)
99.910% <= 1.007 milliseconds (cumulative count 9991)
100.000% <= 1.103 milliseconds (cumulative count 10000)

Summary:
  throughput summary: 181818.19 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.141     0.032     0.127     0.271     0.375     1.063
====== GET ======
  10000 requests completed in 0.07 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.095 milliseconds (cumulative count 5)
50.000% <= 0.143 milliseconds (cumulative count 5050)
75.000% <= 0.239 milliseconds (cumulative count 7860)
87.500% <= 0.263 milliseconds (cumulative count 8974)
93.750% <= 0.295 milliseconds (cumulative count 9402)
96.875% <= 0.351 milliseconds (cumulative count 9703)
98.438% <= 0.383 milliseconds (cumulative count 9844)
99.219% <= 0.423 milliseconds (cumulative count 9928)
99.609% <= 0.479 milliseconds (cumulative count 9963)
99.805% <= 0.519 milliseconds (cumulative count 9991)
99.951% <= 0.527 milliseconds (cumulative count 9999)
99.994% <= 0.535 milliseconds (cumulative count 10000)
100.000% <= 0.535 milliseconds (cumulative count 10000)

Cumulative distribution of latencies:
0.200% <= 0.103 milliseconds (cumulative count 20)
61.970% <= 0.207 milliseconds (cumulative count 6197)
94.320% <= 0.303 milliseconds (cumulative count 9432)
99.030% <= 0.407 milliseconds (cumulative count 9903)
99.740% <= 0.503 milliseconds (cumulative count 9974)
100.000% <= 0.607 milliseconds (cumulative count 10000)

Summary:
  throughput summary: 149253.73 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.182     0.088     0.143     0.319     0.407     0.535

``` 

