#include "RK4Solver.hpp"
#include <cmath>

int main() {
    std::ios_base::sync_with_stdio(0); std::cin.tie(0);

    // Параметры
    double x0 = 0.0, x1 = 2.0;
    Vector<double> u0 = { 1.0 };

    // Уравнение du/dx = 5u
    auto system = [](double x, const Vector<double>& v) {
        return Vector<double>{ 5.0 * v.data[0] }; 
    };

    // Постоянный шаг
    std::cout << "--- Constant Step ---\n";
    RK4Solver<double>::solve(x0, x1, 20, u0, system);

    // Адаптивный шаг
    std::cout << "\n--- Adaptive Step ---\n";
    RK4Solver<double>::Config config;
    config.adaptive = true;
    config.eps = 1e-4;

    RK4Solver<double>::solve(x0, x1, 20, u0, system, config);

    return 0;
}