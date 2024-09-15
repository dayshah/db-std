
#ifndef DBSTD_THREADPOOL
#define DBSTD_THREADPOOL

#include "ringbuffer.hpp"
#include <thread>
#include <functional>

namespace dbstd {

// Single Producer ThreadPool

class SP_ThreadPool {
private:
    RingBuffer<std::function<void()>> jobQueue;
    int numThreads;
    std::vector<std::thread> threads;
    std::mutex mut;
    std::atomic<bool> keepGoing;

    void threadRunner() {
        while (keepGoing) {
            std::optional<std::function<void()>> job;
            {
                std::scoped_lock lock(mut);
                job = jobQueue.dequeue_and_get();
            }
            if (job) (*job)();
        }
    }

public:
    SP_ThreadPool(size_t minQueueCapacity, int numThreads)
    : jobQueue(minQueueCapacity)
    , numThreads(numThreads)
    , threads()
    , keepGoing(true)
    {}

    ~SP_ThreadPool() {
        stop();
    }

    SP_ThreadPool(const SP_ThreadPool&) = delete;
    SP_ThreadPool(SP_ThreadPool&&) = delete;
    SP_ThreadPool& operator=(const SP_ThreadPool&&) = delete;
    SP_ThreadPool& operator=(SP_ThreadPool&&) = delete;

    void start() {
        threads.reserve(numThreads);
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back(&SP_ThreadPool::threadRunner, this);
        }
    }

    void stop() {
        keepGoing = false;
        for (auto& thread : threads) {
            thread.join();
        }
    }

    void enqueueJob(std::function<void()>&& job) {
        std::scoped_lock lock(mut);
        jobQueue.enqueue(std::move(job));
    }

};

}

#endif
 