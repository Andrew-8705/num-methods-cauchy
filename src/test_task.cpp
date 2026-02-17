#include <iostream>
#include <vector>
#include <functional>
#include <cassert>
#include <initializer_list>
#include <iomanip>
#include <cmath>

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

template <typename T>
T calc_h(T a, T b, int n) {
    return (b - a) / static_cast<T>(n);
}

template <typename V, typename T, typename Func>
V calc_rk4_step(T x, const V& v, T h, Func f) {
    V k1 = f(x, v); 
    V k2 = f(x + h * 0.5, v + k1 * (h * 0.5));
    V k3 = f(x + h * 0.5, v + k2 * (h * 0.5));
    V k4 = f(x + h,       v + k3 * h);

    return v + (k1 + k2 * 2.0 + k3 * 2.0 + k4) * (h / 6.0);
}

template <typename V, typename T, typename Func>
void print_results(T x, const V& u, Func exact_sol = nullptr) {
    std::cout << std::setw(8) << x << " | " 
              << std::setw(12) << u.data[0];

    if (exact_sol) {
        T exact_val = exact_sol(x);
        T error = std::abs(u.data[0] - exact_val);
        
        std::cout << " | " << std::setw(12) << exact_val 
                  << " | " << std::setw(12) << error;
    }
    std::cout << "\n";
}

int main() {
    std::ios_base::sync_with_stdio(0); std::cin.tie(0);
    double x_start = 0.0;
    double x_end = 2.0;
    int steps = 20;
    double h = (x_end - x_start) / steps;

    Vector<double> u = { 1.0 }; // u(0) = 1
    double x = x_start;

    auto system = [](double x, const Vector<double>& v) -> Vector<double> {
        return { 5.0 * v.data[0] }; 
    };

    // u(x) = e^(5x)
    auto analytical = [](double x) -> double {
        return std::exp(5.0 * x);
    };

    std::cout << std::fixed << std::setprecision(5);
    std::cout << std::setw(8) << "x" << " | " 
              << std::setw(12) << "RK4" << " | " 
              << std::setw(12) << "Exact" << " | " 
              << std::setw(12) << "Error" << "\n";
    std::cout << std::string(55, '-') << "\n";

    for (int i = 0; i <= steps; ++i) {
        print_results(x, u, analytical);

        if (i < steps) {
            u = calc_rk4_step(x, u, h, system);
            x += h;
        }
    }

    return 0;
}