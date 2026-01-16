// Timer.hpp
#pragma once
#include <chrono>

class Timer
{
public:
    Timer() : start_(std::chrono::high_resolution_clock::now())
    {
    }

    // 返回经过的秒数（浮点型）
    float elapsed() const
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = end - start_;
        return duration.count();
    }

    // 重置计时器
    void reset() { start_ = std::chrono::high_resolution_clock::now(); }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};
