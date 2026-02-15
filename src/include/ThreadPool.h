#pragma once
#include <cstddef>
#include <queue>
#include <vector>

class ThreadPool {
  public:
    ThreadPool(size_t thread = 4);
    ~ThreadPool();
};