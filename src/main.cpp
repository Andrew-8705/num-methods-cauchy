#include "RK4Solver.hpp"
#include <cmath>

int main() {
    std::ios_base::sync_with_stdio(0); std::cin.tie(0);

    // Параметры
    double x0 = 0.0, x1 = 2.0;
    int n = 20;
    Vector<double> u0 = { 1.0 };

    // Уравнение du/dx = 5u
    auto system = [](double x, const Vector<double>& v) {
        return Vector<double>{ 5.0 * v.data[0] }; 
    };

    // Аналитика u(x) = e^(5x)
    auto exact = [](double x) { return std::exp(5.0 * x); };

    RK4Solver<double>::solve(x0, x1, n, u0, system, exact);

    return 0;
}