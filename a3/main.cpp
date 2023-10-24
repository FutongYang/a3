#include "Thread.h"
#include "Lock.h"
#include "iGoodlock.h"
#include "DeadlockFuzzer.h"

int main() {
    Thread t1(1), t2(2), t3(3);
    Lock l1(1), l2(2), l3(3);

    iGoodlock detector;

    std::vector<Thread> threads = { t1, t2, t3 };
    std::vector<Lock> locks = { l1, l2, l3 };

    DeadlockFuzzer fuzzer(detector, threads, locks);

    // Simulate a scenario where a deadlock might occur
    detector.acquire(t1, l1);
    detector.acquire(t2, l2);
    detector.acquire(t3, l3);

    detector.acquire(t1, l2); // t1 is waiting for l2, which is held by t2
    detector.acquire(t2, l3); // t2 is waiting for l3, which is held by t3
    detector.acquire(t3, l1); // t3 is waiting for l1, which is held by t1

    fuzzer.fuzz(); // This should detect a potential deadlock using DeadlockFuzzer

    return 0;
}
