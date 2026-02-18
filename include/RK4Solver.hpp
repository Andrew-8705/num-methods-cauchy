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

    struct Config {
        bool adaptive = false;
        T eps = 1e-5;
        T min_h = 1e-12;
        int max_steps = 10000;
    };

    enum class StepResult {
        Accept,         // Шаг принят, h не меняем
        AcceptIncrease, // Шаг принят, h увеличиваем в 2 раза
        RejectDecrease  // Шаг отклонен, h уменьшаем в 2 раза и пересчитываем
    };

    static StepResult analyze_step(T s_norm, T eps, T h, T min_h) {
        if (s_norm > eps) {
            if (h <= min_h) return StepResult::Accept;
            return StepResult::RejectDecrease;
        } else if (s_norm < eps / 32.0) {
            return StepResult::AcceptIncrease;
        }
        return StepResult::Accept;
    }

    static void solve(T x_start, T x_end, int steps, State u, SystemFunc f, 
        Config config = {false}, std::function<T(T)> exact = nullptr) {
        
        T x = x_start;
        T h = (x_end - x_start) / static_cast<T>(steps);
        
        int cur_step = 0;

        print_header(exact != nullptr);

        while (x < x_end && cur_step < config.max_steps) {

            if (x + h > x_end - std::numeric_limits<T>::epsilon()) {
                h = x_end - x;
            }

            if (!config.adaptive) {
                print_line(x, h, u, exact);
                u = calc_step(x, u, h, f);
                x += h;
            } else {
                State u_full = calc_step(x, u, h, f);
                State u_half_mid = calc_step(x, u, h * 0.5, f);
                State u_half_final = calc_step(x + h * 0.5, u_half_mid, h * 0.5, f);

                T s_norm = ((u_half_final - u_full) * (1.0 / 15.0)).norm();
                
                StepResult res = analyze_step(s_norm, config.eps, h, config.min_h);

                if (res == StepResult::RejectDecrease) {
                    h *= 0.5;
                    continue; // Пересчитываем
                }

                print_line(x, h, u, exact);
                u = u_half_final; 
                x += h;

                if (res == StepResult::AcceptIncrease) {
                    h *= 2.0;
                }
            }
            cur_step++;
        }

        print_line(x, h, u, exact);
    }

private:
    static State calc_step(T x, const State& v, T h, SystemFunc f) {
        State k1 = f(x, v);
        State k2 = f(x + h * 0.5, v + k1 * (h * 0.5));
        State k3 = f(x + h * 0.5, v + k2 * (h * 0.5));
        State k4 = f(x + h,       v + k3 * h);

        return v + (k1 + k2 * 2.0 + k3 * 2.0 + k4) * (h / 6.0);
    }

    static void print_header(bool has_exact) {
        std::cout << std::fixed << std::setprecision(6);

        std::cout << std::setw(10) << "x" << " | " 
                << std::setw(10) << "h" << " | " 
                << std::setw(12) << "RK4";
        
        if (has_exact) {
            std::cout << " | " << std::setw(12) << "Exact" 
                    << " | " << std::setw(12) << "Error";
        }

        std::cout << "\n" << std::string(has_exact ? 73 : 38, '-') << "\n";
    }

    static void print_line(T x, T h, const State& u, std::function<T(T)> exact) {
        std::cout << std::setw(10) << x << " | " 
                << std::setw(10) << h << " | " 
                << std::setw(12) << u.data[0];
                
        if (exact) {
            T val = exact(x);
            std::cout << " | " << std::setw(12) << val 
                    << " | " << std::setw(12) << std::abs(u.data[0] - val);
        }
        std::cout << "\n";
    }
};