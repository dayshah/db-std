
#include <iostream>
#include <ostream>
#include <thread>

#include "vector.hpp"
#include "lifetime.hpp"
#include "ringbuffer.hpp"
#include "threadpool.hpp"

using namespace dbstd;

void vectorTesting() {
    Vector<int> intVec{2};
    intVec.push_back(4);
    intVec.push_back(2);
    intVec.emplace_back(1);

    Vector<Lifetime> lfVec;
    lfVec.push_back(Lifetime{"moved lf"});
    const Lifetime toCopy{"copied lf"};
    lfVec.push_back(toCopy);
    lfVec.reserve(3);
    std::cout << "\nIn place construction*****\n";
    lfVec.emplace_back("emplaced");

    std::cout << "\nfirst vec contents************\n";
    std::cout << lfVec[0] << '\n';
    std::cout << lfVec[1] << '\n';
    std::cout << lfVec[2] << '\n';
    std::cout << "**************************\n\n";

    Vector<Lifetime> copyConstructed{lfVec};
    std::cout << "\ncopy constructed contents************\n";
    std::cout << copyConstructed[0] << '\n';
    std::cout << copyConstructed[1] << '\n';
    std::cout << "**************************\n\n";

    Vector<Lifetime> moveConstructed{std::move(lfVec)};
    std::cout << "\nmove constructed contents************\n";
    std::cout << moveConstructed[0] << '\n';
    std::cout << moveConstructed[1] << '\n';
    std::cout << "**************************\n\n";

    copyConstructed = moveConstructed;
    std::cout << "\ncopy assigned contents************\n";
    std::cout << copyConstructed[0] << '\n';
    std::cout << copyConstructed[1] << '\n';
    std::cout << "**************************\n\n";

    moveConstructed = std::move(copyConstructed);
    std::cout << "\nmove assigned contents************\n";
    std::cout << moveConstructed[0] << '\n';
    std::cout << moveConstructed[1] << '\n';
    std::cout << "**************************\n\n";

}

void dynamicRingBufferTesting() {
    RingBuffer<Lifetime> rbuf(2);
    rbuf.enqueue("first");
    rbuf.enqueue("second");
    auto first = rbuf.dequeue_and_get();
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    std::cout << "first: " << *first << '\n';
    std::cout << "second: " << rbuf.front() << '\n';
    rbuf.enqueue("third");
    std::cout << "failed enqueue: " << (rbuf.enqueue("failed") ? "success" : "fail") << '\n';
    std::cout << "second: " << rbuf.front() << '\n';
    std::cout << "size of two: " << rbuf.size() << '\n';
    rbuf.dequeue();
    auto third = *rbuf.dequeue_and_get();
    std::cout << "third: " << third << '\n';
    // NOLINTEND(bugprone-unchecked-optional-access)
    std::cout << "empty at end: " << (rbuf.empty() ? "true" : "false") << '\n';
}

int main() {
    // vectorTesting();
    // dynamicRingBufferTesting();
    SP_ThreadPool tp(100, 8);
    tp.start();
    for (int i = 0; i < 100; i++) {
        tp.enqueueJob([&i]() { 
            for (int ii = 0; ii < 100000; ii++) {}
            std::cout << "done with " << i << std::endl;
        });
    }
    tp.stop();

    // std::this_thread::sleep_for(std::chrono::nanoseconds(100000));

    return 0;
}
