#pragma once
class Lock {
public:
    explicit Lock(int id) : id(id) {}
    int getId() const { return id; }

private:
    int id;
};
