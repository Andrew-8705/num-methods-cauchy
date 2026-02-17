#pragma once

#include <vector>
#include <initializer_list>
#include <cassert>

template <typename T>
class Vector {
public:
    std::vector<T> data;

    Vector(size_t size) : data(size, T(0)) {}
    Vector(std::initializer_list<T> list) : data(list) {}

    Vector operator+(const Vector& other) const {
        assert(data.size() == other.data.size());
        Vector res(data.size());
        for (size_t i = 0; i < data.size(); ++i) res.data[i] = data[i] + other.data[i];
        return res;
    }

    Vector operator*(T scalar) const {
        Vector res(data.size());
        for (size_t i = 0; i < data.size(); ++i) res.data[i] = data[i] * scalar;
        return res;
    }

    friend Vector operator*(T scalar, const Vector& v) {
        return v * scalar;
    }
};