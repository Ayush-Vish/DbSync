[RUN #1 93%,  56 secs]  8 threads 200 conns:   120029477 ops, 2122002 (avg: 2141609) ops/sec, 150.31MB/sec (avg: 151.76MB/sec), 12[RUN #1 95%,  57 secs]  8 threads 200 conns:   122147602 ops, 2114977 (avg: 2141141) ops/sec, 149.91MB/sec (avg: 151.73MB/sec), 12[RUN #1 97%,  58 secs]  8 threads 200 conns:   124247658 ops, 2098939 (avg: 2140414) ops/sec, 148.86MB/sec (avg: 151.68MB/sec), 12[RUN #1 98%,  59 secs]  8 threads 200 conns:   126382463 ops, 2132672 (avg: 2140283) ops/sec, 151.24MB/sec (avg: 151.67MB/sec), 12[RUN #1 100%,  60 secs]  8 threads 200 conns:   128477104 ops, 2132019 (avg: 2140147) ops/sec, 151.19MB/sec (avg: 151.66MB/sec), 1[RUN #1 100%,  60 secs]  0 threads 200 conns:   128483664 ops, 2132019 (avg: 2140172) ops/sec, 151.19MB/sec (avg: 151.66MB/sec), 11.98 (avg: 11.95) msec latency

8         Threads
200       Connections per thread
60        Seconds


ALL STATS
============================================================================================================================
Type         Ops/sec     Hits/sec   Misses/sec    Avg. Latency     p50 Latency     p99 Latency   p99.9 Latency       KB/sec 
----------------------------------------------------------------------------------------------------------------------------
Sets      1070071.55          ---          ---        11.94821        10.55900        34.04700        60.15900    113524.43 
Gets      1070071.55      2746.76   1067324.79        11.94821        10.55900        34.04700        60.15900     41775.82 
Waits           0.00          ---          ---             ---             ---             ---             ---          --- 
Totals    2140143.09      2746.76   1067324.79        11.94821        10.55900        34.04700        60.15900    155300.25 


Request Latency Distribution
Type     <= msec         Percent
------------------------------------------------------------------------
