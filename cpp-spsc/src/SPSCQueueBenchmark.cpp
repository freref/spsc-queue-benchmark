/*
Copyright (c) 2018 Erik Rigtorp <erik@rigtorp.se>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#include <chrono>
#include <iostream>
#include <rigtorp/SPSCQueue.h>
#include <thread>

#if __has_include(<boost/lockfree/spsc_queue.hpp> )
#include <boost/lockfree/spsc_queue.hpp>
#endif

#if __has_include(<folly/ProducerConsumerQueue.h>)
#include <folly/ProducerConsumerQueue.h>
#endif

int main(int argc, char *argv[])
{
  (void)argc, (void)argv;

  using namespace rigtorp;

  const size_t queueSize = 16777216;
  const int64_t iters = 10000000;

  std::cout << "SPSCQueue:" << std::endl;

  {
    SPSCQueue<int> q(queueSize);
    auto t = std::thread([&]
                         {
      for (int i = 0; i < iters; ++i) {
        while (!q.front())
          ;
        if (*q.front() != i) {
          throw std::runtime_error("");
        }
        q.pop();
      } });

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iters; ++i)
    {
      q.emplace(i);
    }
    t.join();
    auto stop = std::chrono::steady_clock::now();
    std::cout << iters * 1000000 /
                     std::chrono::duration_cast<std::chrono::nanoseconds>(stop -
                                                                          start)
                         .count()
              << " ops/ms" << std::endl;
  }

  {
    SPSCQueue<int> q1(queueSize), q2(queueSize);
    auto t = std::thread([&]
                         {
      for (int i = 0; i < iters; ++i) {
        while (!q1.front())
          ;
        q2.emplace(*q1.front());
        q1.pop();
      } });

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iters; ++i)
    {
      q1.emplace(i);
      while (!q2.front())
        ;
      q2.pop();
    }
    auto stop = std::chrono::steady_clock::now();
    t.join();
    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(stop -
                                                                      start)
                         .count() /
                     iters
              << " ns RTT" << std::endl;
  }

#if __has_include(<boost/lockfree/spsc_queue.hpp> )
  std::cout << "boost::lockfree::spsc:" << std::endl;
  {
    boost::lockfree::spsc_queue<int> q(queueSize);
    auto t = std::thread([&]
                         {
      for (int i = 0; i < iters; ++i) {
        int val;
        while (q.pop(&val, 1) != 1)
          ;
        if (val != i) {
          throw std::runtime_error("");
        }
      } });

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iters; ++i)
    {
      while (!q.push(i))
        ;
    }
    t.join();
    auto stop = std::chrono::steady_clock::now();
    std::cout << iters * 1000000 /
                     std::chrono::duration_cast<std::chrono::nanoseconds>(stop -
                                                                          start)
                         .count()
              << " ops/ms" << std::endl;
  }

  {
    boost::lockfree::spsc_queue<int> q1(queueSize), q2(queueSize);
    auto t = std::thread([&]
                         {
      for (int i = 0; i < iters; ++i) {
        int val;
        while (q1.pop(&val, 1) != 1)
          ;
        while (!q2.push(val))
          ;
      } });

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iters; ++i)
    {
      while (!q1.push(i))
        ;
      int val;
      while (q2.pop(&val, 1) != 1)
        ;
    }
    auto stop = std::chrono::steady_clock::now();
    t.join();
    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(stop -
                                                                      start)
                         .count() /
                     iters
              << " ns RTT" << std::endl;
  }
#endif

#if __has_include(<folly/ProducerConsumerQueue.h>)
  std::cout << "folly::ProducerConsumerQueue:" << std::endl;

  {
    folly::ProducerConsumerQueue<int> q(queueSize);
    auto t = std::thread([&]
                         {
      for (int i = 0; i < iters; ++i) {
        int val;
        while (!q.read(val))
          ;
        if (val != i) {
          throw std::runtime_error("");
        }
      } });

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iters; ++i)
    {
      while (!q.write(i))
        ;
    }
    t.join();
    auto stop = std::chrono::steady_clock::now();
    std::cout << iters * 1000000 /
                     std::chrono::duration_cast<std::chrono::nanoseconds>(stop -
                                                                          start)
                         .count()
              << " ops/ms" << std::endl;
  }

  {
    folly::ProducerConsumerQueue<int> q1(queueSize), q2(queueSize);
    auto t = std::thread([&]
                         {
        for (int i = 0; i < iters; ++i) {
        int val;
        while (!q1.read(val))
          ;
        q2.write(val);
      } });

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iters; ++i)
    {
      while (!q1.write(i))
        ;
      int val;
      while (!q2.read(val))
        ;
    }
    auto stop = std::chrono::steady_clock::now();
    t.join();
    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(stop -
                                                                      start)
                         .count() /
                     iters
              << " ns RTT" << std::endl;
  }
#endif

  return 0;
}
