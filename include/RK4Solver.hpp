#pragma once

#include "Vector.hpp"
#include <functional>
#include <iostream>
#include <iomanip>

template <typename T>
class RK4Solver {
public:
    using State = Vector<T>;
    using SystemFunc = std::function<State(T, const State&)>;

    static void solve(T x_start, T x_end, int steps, State u, SystemFunc f, std::function<T(T)> exact = nullptr) {
        T h = (x_end - x_start) / static_cast<T>(steps);
        T x = x_start;

        print_header(exact != nullptr);

        for (int i = 0; i <= steps; ++i) {
            print_line(x, u, exact);
            if (i < steps) {
                u = calc_step(x, u, h, f);
                x += h;
            }
        }
    }

private:
    static State calc_step(T x, const State& v, T h, SystemFunc f) {
        State k1 = f(x, v);
        State k2 = f(x + h * 0.5, v + k1 * (h * 0.5));
        State k3 = f(x + h * 0.5, v + k2 * (h * 0.5));
        State k4 = f(x + h,       v + k3 * h);

        return v + (k1 + k2 * 2.0 + k3 * 2.0 + k4) * (h / 6.0);
    }

    // static T calc_h(T a, T b, int n) {
    //     return (b - a) / static_cast<T>(n);
    // }


    static void print_header(bool has_exact) {
        std::cout << std::fixed << std::setprecision(5);
        std::cout << std::setw(8) << "x" << " | " << std::setw(12) << "RK4";
        if (has_exact) std::cout << " | " << std::setw(12) << "Exact" << " | " << std::setw(12) << "Error";
        std::cout << "\n" << std::string(has_exact ? 55 : 25, '-') << "\n";
    }

    static void print_line(T x, const State& u, std::function<T(T)> exact) {
        std::cout << std::setw(8) << x << " | " << std::setw(12) << u.data[0];
        if (exact) {
            T val = exact(x);
            std::cout << " | " << std::setw(12) << val << " | " << std::setw(12) << std::abs(u.data[0] - val);
        }
        std::cout << "\n";
    }
};