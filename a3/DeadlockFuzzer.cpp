#include "DeadlockFuzzer.h"
#include <iostream>
#include <pthread.h>
#include <omp.h>

struct ThreadData {
    int threadId;
    // Add other necessary fields here
};

DeadlockFuzzer::DeadlockFuzzer(iGoodlock& goodlock, const std::vector<Thread>& threads, const std::vector<Lock>& locks)
    : goodlock(goodlock), threads(threads), locks(locks), gen(rd()) {}

int DeadlockFuzzer::getRandomThreadIndex(const std::unordered_set<int>& paused) {
    std::vector<int> enabledThreads;
    for (int i = 0; i < threads.size(); ++i) {
        if (paused.find(threads[i].getId()) == paused.end()) {
            enabledThreads.push_back(i);
        }
    }

    if (enabledThreads.empty()) {
        return -1;
    }

    std::uniform_int_distribution<> dis(0, enabledThreads.size() - 1);
    return enabledThreads[dis(gen)];
}

void DeadlockFuzzer::executeStatement(const Thread& thread, std::unordered_map<int, std::stack<int>>& lockSet,
    std::unordered_map<int, std::stack<int>>& context, std::unordered_set<int>& paused) {
    // Simulate the execution of a statement by the thread
    std::cout << "Executing statement by thread " << thread.getId() << std::endl;

    // For simplicity, let's assume each thread performs a sequence of acquire and release operations
    // on locks. The sequence of operations for each thread is predefined in a map.
    // In a real-world scenario, this sequence would be determined by the program being executed.
    static std::unordered_map<int, std::vector<std::pair<std::string, int>>> operations = {
        {1, {{"acquire", 1}, {"acquire", 2}, {"release", 1}, {"release", 2}}},
        {2, {{"acquire", 2}, {"acquire", 3}, {"release", 2}, {"release", 3}}},
        {3, {{"acquire", 3}, {"acquire", 1}, {"release", 3}, {"release", 1}}}
    };

    // Get the next operation for the thread
    auto& ops = operations[thread.getId()];
    if (!ops.empty()) {
        std::pair<std::string, int> operation = ops.front();
        ops.erase(ops.begin());

        std::string op = operation.first;
        int lockId = operation.second;

        if (op == "acquire") {
            lockSet[thread.getId()].push(lockId);
            context[thread.getId()].push(lockId);  // Assuming context is represented by lockId for simplicity
        }
        else if (op == "release") {
            lockSet[thread.getId()].pop();
            context[thread.getId()].pop();
        }

        // Check for real deadlocks after each operation
        checkRealDeadlock(lockSet);
    }
    else {
        // If the thread has no more operations, consider it paused
        paused.insert(thread.getId());
    }
}

void DeadlockFuzzer::checkRealDeadlock(const std::unordered_map<int, std::stack<int>>& lockSet) {
    // Convert the lock set stacks to vectors for easier iteration
    std::unordered_map<int, std::vector<int>> lockVectors;
    for (const auto& threadLockPair : lockSet) {
        int threadId = threadLockPair.first;
        std::stack<int> lockStack = threadLockPair.second;

        std::vector<int> locks;
        while (!lockStack.empty()) {
            locks.push_back(lockStack.top());
            lockStack.pop();
        }
        std::reverse(locks.begin(), locks.end());
        lockVectors[threadId] = locks;
    }

    // Check for the deadlock condition described in Algorithm 4
#pragma omp parallel for collapse(2)
    for (const auto& threadLockPair1 : lockVectors) {
        int threadId1 = threadLockPair1.first;
        const std::vector<int>& locks1 = threadLockPair1.second;

        for (const auto& threadLockPair2 : lockVectors) {
            int threadId2 = threadLockPair2.first;
            const std::vector<int>& locks2 = threadLockPair2.second;

            if (threadId1 != threadId2) {
                for (size_t i = 0; i < locks1.size(); ++i) {
                    for (size_t j = 0; j < locks2.size(); ++j) {
                        if (locks1[i] == locks2[j] && i < j) {
#pragma omp critical
                            {
                                std::cout << "Real Deadlock Found between threads " << threadId1 << " and " << threadId2 << " on lock " << locks1[i] << std::endl;
                            }
                            return;
                        }
                    }
                }
            }
        }
    }
}

void DeadlockFuzzer::fuzz() {
    std::unordered_set<int> paused;
    std::unordered_map<int, std::stack<int>> lockSet;
    std::unordered_map<int, std::stack<int>> context;

    while (!threads.empty()) {
        int threadIndex = getRandomThreadIndex(paused);
        if (threadIndex == -1) {
            std::cout << "System Stall!" << std::endl;
            break;
        }

        Thread thread = threads[threadIndex];
        // Simulate the execution of statements by the thread
        executeStatement(thread, lockSet, context, paused);
    }

    if (!threads.empty()) {
        std::cout << "System Stall!" << std::endl;
    }
}
