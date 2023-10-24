#include "iGoodlock.h"
#include <omp.h>

void iGoodlock::acquire(const Thread& thread, const Lock& lock) {
#pragma omp critical
    {
        lockToThread[lock.getId()] = thread.getId();
        threadToLocks[thread.getId()].insert(lock.getId());
    }
}

void iGoodlock::release(const Thread& thread, const Lock& lock) {
#pragma omp critical
    {
        threadToLocks[thread.getId()].erase(lock.getId());
        if (threadToLocks[thread.getId()].empty()) {
            threadToLocks.erase(thread.getId());
        }
        lockToThread.erase(lock.getId());
    }
}

void iGoodlock::checkForDeadlocks() const {
    std::vector<std::pair<int, std::set<int>>> threadLockPairs(threadToLocks.begin(), threadToLocks.end());
    int n = static_cast<int>(threadLockPairs.size());

#pragma omp parallel for collapse(2)
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            const auto& [threadId1, locks1] = threadLockPairs[i];
            const auto& [threadId2, locks2] = threadLockPairs[j];

            if (threadId1 != threadId2) {
                // Check if the threads are waiting on each other's locks
                for (int lockId1 : locks1) {
                    if (locks2.count(lockId1) > 0) {
#pragma omp critical
                        {
                            std::cout << "Potential deadlock detected between threads "
                                << threadId1 << " and " << threadId2 << std::endl;
                            std::cout << "Thread " << threadId1 << " is waiting for lock "
                                << lockId1 << " held by thread " << threadId2 << std::endl;
                            std::cout << "Thread " << threadId2 << " is waiting for lock "
                                << lockId1 << " held by thread " << threadId1 << std::endl;
                        }
                    }
                }
            }
        }
    }
}



