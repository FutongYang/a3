#pragma once
#include <iostream>
#include <map>
#include <set>
#include "Thread.h"
#include "Lock.h"

class iGoodlock {
public:
    void acquire(const Thread& thread, const Lock& lock);
    void release(const Thread& thread, const Lock& lock);
    void checkForDeadlocks() const;

private:
    std::map<int, int> lockToThread; // Maps lock IDs to the IDs of the threads that hold them
    std::map<int, std::set<int>> threadToLocks; // Maps thread IDs to the sets of locks they hold
};
