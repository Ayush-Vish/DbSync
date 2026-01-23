```
.
{22:37}~/Desktop/DB:master ✗ ➭ redis-benchmark -p 6379 -t set,get -n 10000 -c 50                                            
====== SET ======                                          
  10000 requests completed in 0.05 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1
  host configuration "save": 3600 1 300 100 60 10000
  host configuration "appendonly": no
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.039 milliseconds (cumulative count 1)
50.000% <= 0.127 milliseconds (cumulative count 5526)
75.000% <= 0.135 milliseconds (cumulative count 7824)
87.500% <= 0.167 milliseconds (cumulative count 8763)
93.750% <= 0.255 milliseconds (cumulative count 9432)
96.875% <= 0.295 milliseconds (cumulative count 9729)
98.438% <= 0.343 milliseconds (cumulative count 9859)
99.219% <= 0.431 milliseconds (cumulative count 9927)
99.609% <= 0.543 milliseconds (cumulative count 9962)
99.805% <= 0.647 milliseconds (cumulative count 9981)
99.902% <= 0.695 milliseconds (cumulative count 9992)
99.951% <= 0.719 milliseconds (cumulative count 9998)
99.988% <= 0.727 milliseconds (cumulative count 9999)
99.994% <= 0.735 milliseconds (cumulative count 10000)
100.000% <= 0.735 milliseconds (cumulative count 10000)

Cumulative distribution of latencies:
0.500% <= 0.103 milliseconds (cumulative count 50)
90.740% <= 0.207 milliseconds (cumulative count 9074)
97.650% <= 0.303 milliseconds (cumulative count 9765)
98.920% <= 0.407 milliseconds (cumulative count 9892)
99.560% <= 0.503 milliseconds (cumulative count 9956)
99.730% <= 0.607 milliseconds (cumulative count 9973)
99.940% <= 0.703 milliseconds (cumulative count 9994)
100.000% <= 0.807 milliseconds (cumulative count 10000)

Summary:
  throughput summary: 185185.19 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.145     0.032     0.127     0.271     0.415     0.735
====== GET ======
  10000 requests completed in 0.05 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1
  host configuration "save": 3600 1 300 100 60 10000
  host configuration "appendonly": no
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.047 milliseconds (cumulative count 4)
50.000% <= 0.127 milliseconds (cumulative count 6057)
75.000% <= 0.135 milliseconds (cumulative count 7934)
87.500% <= 0.151 milliseconds (cumulative count 8962)
93.750% <= 0.191 milliseconds (cumulative count 9421)
96.875% <= 0.239 milliseconds (cumulative count 9692)
98.438% <= 0.303 milliseconds (cumulative count 9854)
99.219% <= 0.343 milliseconds (cumulative count 9936)
99.609% <= 0.359 milliseconds (cumulative count 9963)
99.805% <= 0.391 milliseconds (cumulative count 9993)
99.951% <= 0.423 milliseconds (cumulative count 9996)
99.976% <= 0.447 milliseconds (cumulative count 9998)
99.988% <= 0.527 milliseconds (cumulative count 9999)
99.994% <= 0.535 milliseconds (cumulative count 10000)
100.000% <= 0.535 milliseconds (cumulative count 10000)

Cumulative distribution of latencies:
1.080% <= 0.103 milliseconds (cumulative count 108)
95.330% <= 0.207 milliseconds (cumulative count 9533)
98.540% <= 0.303 milliseconds (cumulative count 9854)
99.940% <= 0.407 milliseconds (cumulative count 9994)
99.980% <= 0.503 milliseconds (cumulative count 9998)
100.000% <= 0.607 milliseconds (cumulative count 10000)

Summary:
  throughput summary: 192307.69 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.135     0.040     0.127     0.207     0.335     0.535

{22:37}~/Desktop/DB:master ✗ ➭ 

```
