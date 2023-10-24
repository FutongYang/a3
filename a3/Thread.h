#pragma once
#include <vector>
#include <string>

class Lock;

class Thread {
public:
    explicit Thread(int id) : id(id) {}
    int getId() const { return id; }

private:
    int id;
};