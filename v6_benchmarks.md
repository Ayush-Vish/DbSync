{13:16}~/Desktop/DB:main ✗ ➭ redis-benchmark -t set,get -n 2000000 -c 100 -P 32 --threads 8
WARNING: Could not fetch server CONFIG
====== SET ======                                                       
  2000000 requests completed in 0.51 seconds
  100 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: yes
  threads: 8

Latency by percentile distribution:
0.000% <= 0.023 milliseconds (cumulative count 128)
50.000% <= 0.199 milliseconds (cumulative count 1008288)
75.000% <= 0.343 milliseconds (cumulative count 1509952)
87.500% <= 0.503 milliseconds (cumulative count 1750272)
93.750% <= 0.719 milliseconds (cumulative count 1877248)
96.875% <= 0.999 milliseconds (cumulative count 1937792)
98.438% <= 1.743 milliseconds (cumulative count 1968800)
99.219% <= 2.695 milliseconds (cumulative count 1984448)
99.609% <= 3.383 milliseconds (cumulative count 1992192)
99.805% <= 3.871 milliseconds (cumulative count 1996128)
99.902% <= 4.151 milliseconds (cumulative count 1998112)
99.951% <= 4.463 milliseconds (cumulative count 1999104)
99.976% <= 4.775 milliseconds (cumulative count 1999520)
99.988% <= 5.407 milliseconds (cumulative count 1999776)
99.994% <= 5.847 milliseconds (cumulative count 1999904)
99.997% <= 6.463 milliseconds (cumulative count 1999968)
99.998% <= 6.607 milliseconds (cumulative count 2000000)
100.000% <= 6.607 milliseconds (cumulative count 2000000)

Cumulative distribution of latencies:
9.229% <= 0.103 milliseconds (cumulative count 184576)
52.358% <= 0.207 milliseconds (cumulative count 1047168)
70.349% <= 0.303 milliseconds (cumulative count 1406976)
81.349% <= 0.407 milliseconds (cumulative count 1626976)
87.514% <= 0.503 milliseconds (cumulative count 1750272)
91.470% <= 0.607 milliseconds (cumulative count 1829408)
93.608% <= 0.703 milliseconds (cumulative count 1872160)
95.141% <= 0.807 milliseconds (cumulative count 1902816)
96.216% <= 0.903 milliseconds (cumulative count 1924320)
96.941% <= 1.007 milliseconds (cumulative count 1938816)
97.382% <= 1.103 milliseconds (cumulative count 1947648)
97.717% <= 1.207 milliseconds (cumulative count 1954336)
97.941% <= 1.303 milliseconds (cumulative count 1958816)
98.102% <= 1.407 milliseconds (cumulative count 1962048)
98.211% <= 1.503 milliseconds (cumulative count 1964224)
98.318% <= 1.607 milliseconds (cumulative count 1966368)
98.405% <= 1.703 milliseconds (cumulative count 1968096)
98.515% <= 1.807 milliseconds (cumulative count 1970304)
98.614% <= 1.903 milliseconds (cumulative count 1972288)
98.698% <= 2.007 milliseconds (cumulative count 1973952)
98.778% <= 2.103 milliseconds (cumulative count 1975552)
99.464% <= 3.103 milliseconds (cumulative count 1989280)
99.891% <= 4.103 milliseconds (cumulative count 1997824)
99.982% <= 5.103 milliseconds (cumulative count 1999648)
99.997% <= 6.103 milliseconds (cumulative count 1999936)
100.000% <= 7.103 milliseconds (cumulative count 2000000)

Summary:
  throughput summary: 3952569.25 requests per second
  latency summary (msec):
          avg       mink       p50       p95       p99       max
        0.311     0.016     0.199     0.799     2.399     6.607
====== GET ======                                                       
  2000000 requests completed in 0.51 seconds
  100 parallel clients
  3 bytes payload
  keep alive: 1
  multi-thread: yes
  threads: 8

Latency by percentile distribution:
0.000% <= 0.031 milliseconds (cumulative count 352)
50.000% <= 0.207 milliseconds (cumulative count 1025760)
75.000% <= 0.359 milliseconds (cumulative count 1509856)
87.500% <= 0.583 milliseconds (cumulative count 1753312)
93.750% <= 0.863 milliseconds (cumulative count 1875200)
96.875% <= 1.199 milliseconds (cumulative count 1938176)
98.438% <= 1.623 milliseconds (cumulative count 1969024)
99.219% <= 2.271 milliseconds (cumulative count 1984416)
99.609% <= 3.455 milliseconds (cumulative count 1992192)
99.805% <= 4.071 milliseconds (cumulative count 1996160)
99.902% <= 4.383 milliseconds (cumulative count 1998048)
99.951% <= 5.159 milliseconds (cumulative count 1999072)
99.976% <= 5.415 milliseconds (cumulative count 1999520)
99.988% <= 5.799 milliseconds (cumulative count 1999776)
99.994% <= 7.207 milliseconds (cumulative count 1999904)
99.997% <= 7.983 milliseconds (cumulative count 1999968)
99.998% <= 9.655 milliseconds (cumulative count 2000000)
100.000% <= 9.655 milliseconds (cumulative count 2000000)

Cumulative distribution of latencies:
7.104% <= 0.103 milliseconds (cumulative count 142080)
51.288% <= 0.207 milliseconds (cumulative count 1025760)
69.488% <= 0.303 milliseconds (cumulative count 1389760)
79.155% <= 0.407 milliseconds (cumulative count 1583104)
84.488% <= 0.503 milliseconds (cumulative count 1689760)
88.469% <= 0.607 milliseconds (cumulative count 1769376)
91.048% <= 0.703 milliseconds (cumulative count 1820960)
92.979% <= 0.807 milliseconds (cumulative count 1859584)
94.270% <= 0.903 milliseconds (cumulative count 1885408)
95.365% <= 1.007 milliseconds (cumulative count 1907296)
96.206% <= 1.103 milliseconds (cumulative count 1924128)
96.950% <= 1.207 milliseconds (cumulative count 1939008)
97.448% <= 1.303 milliseconds (cumulative count 1948960)
97.821% <= 1.407 milliseconds (cumulative count 1956416)
98.109% <= 1.503 milliseconds (cumulative count 1962176)
98.411% <= 1.607 milliseconds (cumulative count 1968224)
98.666% <= 1.703 milliseconds (cumulative count 1973312)
98.893% <= 1.807 milliseconds (cumulative count 1977856)
99.022% <= 1.903 milliseconds (cumulative count 1980448)
99.085% <= 2.007 milliseconds (cumulative count 1981696)
99.136% <= 2.103 milliseconds (cumulative count 1982720)
99.475% <= 3.103 milliseconds (cumulative count 1989504)
99.819% <= 4.103 milliseconds (cumulative count 1996384)
99.949% <= 5.103 milliseconds (cumulative count 1998976)
99.989% <= 6.103 milliseconds (cumulative count 1999776)
99.994% <= 7.103 milliseconds (cumulative count 1999872)
99.998% <= 8.103 milliseconds (cumulative count 1999968)
100.000% <= 10.103 milliseconds (cumulative count 2000000)

Summary:
  throughput summary: 3944773.00 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.333     0.024     0.207     0.975     1.895     9.655
