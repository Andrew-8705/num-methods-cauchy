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

    struct StepInfo {
        int iter;           // Номер итерации
        T x;                // Текущее время
        T h;                // Текущий шаг
        State v;            // Значение с полным шагом (v_i)
        State v2;           // Значение с двумя половинными (v_2i)
        T olp;              // Оценка локальной погрешности
        int c1;             // Счетчик делений шага на этом этапе
        int c2;             // Счетчик удвоений шага на этом этапе
        T error_exact;      // Глобальная ошибка (если есть точное решение)
    };

    using OutputCallback = std::function<void(const StepInfo& info)>;

    struct Config {
        bool adaptive = false;
        T eps = 1e-5;
        T min_h = 1e-12;
        int max_steps = 10000;
        T initial_h = 0.01;
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

    static void solve(T x_start, T x_end, State u, SystemFunc f,
                     OutputCallback callback, Config config,
                     std::function<State(T)> exactFunc = nullptr) {
        
        T x = x_start;
        T h = config.initial_h;
        int iter = 0;

        StepInfo startInfo = {iter, x, h, u, u, 0, 0, 0, 0};
        if(exactFunc) startInfo.error_exact = (u - exactFunc(x)).norm();
        if(callback) callback(startInfo);

        while (x < x_end - 1e-9 && iter < config.max_steps) {
            // Выход на правую границу
            if (x + h > x_end) h = x_end - x;

            // State v_full = calc_step(x, u, h, f);
            // State v_half_1 = calc_step(x, u, h * 0.5, f);
            // State v_half_2 = calc_step(x + h * 0.5, v_half_1, h * 0.5, f); // v2i

            // State diff = v_half_2 - v_full;
            // T s_norm = diff.norm() / 15.0;

            int c1 = 0; // Счётчик деления
            int c2 = 0; // Счётчик удвоения
            bool step_accepted = false;

            State v_full(u.data.size());
            State v_half_2(u.data.size());
            T s_norm = 0;

            while (!step_accepted) {

                v_full = calc_step(x, u, h, f);
                State v_half_1 = calc_step(x, u, h * 0.5, f);
                v_half_2 = calc_step(x + h * 0.5, v_half_1, h * 0.5, f); // v2i;

                // OLP = (v_half - v_full) / (2^p - 1)
                State diff = v_half_2 - v_full;
                s_norm = diff.norm() / 15.0;

                // Если адаптивность выключена, всегда принимаем
                if (!config.adaptive) {
                    step_accepted = true;
                    break;
                }

                StepResult res = analyze_step(s_norm, config.eps, h, config.min_h);

                if (res == StepResult::RejectDecrease) {
                    h *= 0.5;
                    c1++;
                } else {
                    step_accepted = true;
                    if (res == StepResult::AcceptIncrease) {
                        c2 = 1; // Помечаем, что следующий шаг будет больше
                    }
                }
            }

            iter++;
            x += h;
            u = v_half_2; // Берём результат с половинным шагом, поскольку он точнее и уже вычислен

            T err = (exactFunc) ? (u - exactFunc(x)).norm() : 0;

            StepInfo info = {
                iter,
                x,
                h,
                v_full,
                v_half_2,
                s_norm, // olp
                c1,
                c2,
                err // error_exact
            };

            if (callback) callback(info);

            if (c2 > 0) {
                h *= 2.0;
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

    // static void print_header(bool has_exact) {
    //     std::cout << std::fixed << std::setprecision(6);

    //     std::cout << std::setw(10) << "x" << " | " 
    //             << std::setw(10) << "h" << " | " 
    //             << std::setw(12) << "RK4";
        
    //     if (has_exact) {
    //         std::cout << " | " << std::setw(12) << "Exact" 
    //                 << " | " << std::setw(12) << "Error";
    //     }

    //     std::cout << "\n" << std::string(has_exact ? 73 : 38, '-') << "\n";
    // }

    // static void print_line(T x, T h, const State& u, std::function<T(T)> exact) {
    //     std::cout << std::setw(10) << x << " | " 
    //             << std::setw(10) << h << " | " 
    //             << std::setw(12) << u.data[0];
                
    //     if (exact) {
    //         T val = exact(x);
    //         std::cout << " | " << std::setw(12) << val 
    //                 << " | " << std::setw(12) << std::abs(u.data[0] - val);
    //     }
    //     std::cout << "\n";
    // }
};
