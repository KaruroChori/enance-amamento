#pragma once

/**
 * @file time-series.hpp
 * @author karurochari
 * @brief Utility structures for limited data buffers, used to show graphs
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <cmath>
#include <glm/glm.hpp>
#include <vector>

template<uint SAMPLES=2048>
struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    std::vector<glm::vec2> Data;
    ScrollingBuffer(int max_size = SAMPLES) {
        MaxSize = max_size;
        Offset  = 0;
        Data.resize(MaxSize);
    }
    void AddPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(glm::vec2(x,y));
        else {
            Data[Offset] = glm::vec2(x,y);
            Offset =  (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.resize(0);
            Offset  = 0;
        }
    }
};

// utility structure for realtime plot
template<uint SAMPLES=2048>
struct RollingBuffer {
    float Span;
    std::vector<glm::vec2> Data;
    RollingBuffer() {
        Span = 10.0f;
        Data.reserve(SAMPLES);
    }
    void AddPoint(float x, float y) {
        float xmod = fmodf(x, Span);
        if (!Data.empty() && xmod < Data.back().x)
            Data.resize(0);
        Data.push_back(glm::vec2(xmod, y));
    }
};