#pragma once
#pragma once
#include "iGoodlock.h"
#include "Thread.h"
#include "Lock.h"
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <random>

class DeadlockFuzzer {
public:
    DeadlockFuzzer(iGoodlock& goodlock, const std::vector<Thread>& threads, const std::vector<Lock>& locks);
    void fuzz();
private:
    iGoodlock& goodlock;
    std::vector<Thread> threads;
    std::vector<Lock> locks;
    std::random_device rd;
    std::mt19937 gen;

    int getRandomThreadIndex(const std::unordered_set<int>& paused);
    void executeStatement(const Thread& thread, std::unordered_map<int, std::stack<int>>& lockSet,
        std::unordered_map<int, std::stack<int>>& context, std::unordered_set<int>& paused);
    void checkRealDeadlock(const std::unordered_map<int, std::stack<int>>& lockSet);
};
