```
GET: rps=0.0 (overall: 93.6) avg_msec=-nan (overa                                                 GET: rps=0.0 (overall: 93.3) avg_msec=-nan (overa                                                 GET: rps=0.0 (overall: 93.1) avg_msec=-nan (overa                                                 GET: rps=0.0 (overall: 92.9) avg_msec=-nan (overaGET: rps=0.0 (overall: 66.1) avg_msec=-nan (overall: 0.012) 
====== GET ======                                           
  10000 requests completed in 536.26 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.007 milliseconds (cumulative count 2127)
50.000% <= 0.015 milliseconds (cumulative count 7957)
87.500% <= 0.023 milliseconds (cumulative count 9869)
99.219% <= 0.031 milliseconds (cumulative count 9943)
99.609% <= 0.047 milliseconds (cumulative count 9961)
99.805% <= 0.087 milliseconds (cumulative count 9984)
99.902% <= 0.103 milliseconds (cumulative count 9991)
99.951% <= 0.183 milliseconds (cumulative count 9996)
99.976% <= 0.207 milliseconds (cumulative count 9998)
99.988% <= 0.231 milliseconds (cumulative count 9999)
99.994% <= 0.279 milliseconds (cumulative count 10000)
100.000% <= 0.279 milliseconds (cumulative count 10000)

Cumulative distribution of latencies:
99.910% <= 0.103 milliseconds (cumulative count 9991)
99.980% <= 0.207 milliseconds (cumulative count 9998)
100.000% <= 0.303 milliseconds (cumulative count 10000)

Summary:
  throughput summary: 18.65 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.012     0.000     0.015     0.023     0.031     0.279

{13:59}~/Desktop/DB:master ✓ ➭ 
{14:32}~/Desktop/DB:master ✓ ➭ redis-benchmark -p 6379 -t set,get -n 10000 -c 50~
 *  History restored 

{22:30}~/Desktop/DB:master ✗ ➭ npm run dev 
{22:30}~/Desktop/DB:master ✗ ➭ redis-benchmark -p 6379 -t set,get -n 10000 -c 50 
WARNING: Could not fetch server CONFIG
SET: rps=0.0 (overall: 94.4) avg_msec=-nan (overall: 0.051)         
SET: rps=0.0 (overall: 94.2) avg_msec=-nan (overall: 0.051) 
SET: rps=0.0 (overall: 94.0) avg_msec=-nan (overall: 0.051) 
SET: rps=0.0 (overall: 93.7) avg_msec=-nan (overall: 0.051) 
SET: rps=0.0 (overall: 93.5) avg_msec=-nan (overall: 0.051) 
SET: rps=0.0 (overall: 93.3) avg_msec=-nan (overall: 0.051) 

====== SET ======                                            
  10000 requests completed in 136.52 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.015 milliseconds (cumulative count 1)
50.000% <= 0.047 milliseconds (cumulative count 7243)
75.000% <= 0.055 milliseconds (cumulative count 8559)
87.500% <= 0.079 milliseconds (cumulative count 8972)
93.750% <= 0.087 milliseconds (cumulative count 9445)
96.875% <= 0.095 milliseconds (cumulative count 9706)
98.438% <= 0.111 milliseconds (cumulative count 9880)
99.219% <= 0.151 milliseconds (cumulative count 9937)
99.609% <= 0.223 milliseconds (cumulative count 9961)
99.805% <= 0.423 milliseconds (cumulative count 9981)
99.902% <= 0.655 milliseconds (cumulative count 9991)
99.951% <= 0.687 milliseconds (cumulative count 9997)
99.976% <= 0.695 milliseconds (cumulative count 9998)
99.988% <= 0.703 milliseconds (cumulative count 9999)
99.994% <= 0.711 milliseconds (cumulative count 10000)
100.000% <= 0.711 milliseconds (cumulative count 10000)

Cumulative distribution of latencies:
98.350% <= 0.103 milliseconds (cumulative count 9835)
99.590% <= 0.207 milliseconds (cumulative count 9959)
99.730% <= 0.303 milliseconds (cumulative count 9973)
99.740% <= 0.407 milliseconds (cumulative count 9974)
99.830% <= 0.503 milliseconds (cumulative count 9983)
99.860% <= 0.607 milliseconds (cumulative count 9986)
99.990% <= 0.703 milliseconds (cumulative count 9999)
100.000% <= 0.807 milliseconds (cumulative count 10000)

Summary:
  throughput summary: 73.25 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.051     0.008     0.047     0.095     0.127     0.711
GET: rps=0.0 (overall: 78.8) avg_msec=-nan (overall: 0.051)          
GET: rps=0.0 (overall: 78.4) avg_msec=-nan (overall: 0.051) 
GET: rps=0.0 (overall: 78.2) avg_msec=-nan (overall: 0.051) 
====== GET ======                                           
  10002 requests completed in 135.99 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: no

Latency by percentile distribution:
0.000% <= 0.023 milliseconds (cumulative count 6)
50.000% <= 0.055 milliseconds (cumulative count 8879)
93.750% <= 0.063 milliseconds (cumulative count 9763)
98.438% <= 0.087 milliseconds (cumulative count 9850)
99.219% <= 0.151 milliseconds (cumulative count 9922)
99.609% <= 0.183 milliseconds (cumulative count 9973)
99.805% <= 0.191 milliseconds (cumulative count 9985)
99.902% <= 0.207 milliseconds (cumulative count 9991)
99.951% <= 0.223 milliseconds (cumulative count 9996)
99.976% <= 0.231 milliseconds (cumulative count 9998)
99.988% <= 0.239 milliseconds (cumulative count 10000)
100.000% <= 0.239 milliseconds (cumulative count 10000)

Cumulative distribution of latencies:
98.740% <= 0.103 milliseconds (cumulative count 9874)
99.910% <= 0.207 milliseconds (cumulative count 9991)
100.000% <= 0.303 milliseconds (cumulative count 10000)

Summary:
  throughput summary: 73.55 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.051     0.016     0.055     0.063     0.127     0.239
```

