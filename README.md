# spsc-queue-benchmark
Benchmarking suite for various single producer single consumer (SPSC) queue implementations. Feel free to add more implementations and open a PR.

We are currently benchmarking the following implementations:
- [freref/spsc-queue](https://github.com/freref/spsc-queue)
- [*boost::lockfree::spsc*](https://www.boost.org/doc/libs/1_76_0/doc/html/boost/lockfree/spsc_queue.html)
- [*folly::ProducerConsumerQueue*](https://github.com/facebook/folly/blob/master/folly/docs/ProducerConsumerQueue.md)
- [rigtorp/SPSCQueue](https://github.com/rigtorp/SPSCQueue/tree/master)
- [cdolan/zig-spsc-ring](https://github.com/cdolan/zig-spsc-ring.git)
## Results
The benchmarks were run on a MacBook Pro (Apple M4 Pro, 14 cores: 10 performance + 4 efficiency) with 48 GB unified memory.
![Benchmark bar chart](./benchmarks.png)
