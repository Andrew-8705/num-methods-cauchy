#include "TaskSolver.h"

#include <cmath>

SimulationResults TaskSolver::solveTest(const TestParams& p) {
    SimulationResults results;

    // Формула: u' = A*u, A = (-1)^N * N/2
    double A = std::pow(-1, p.N) * (double(p.N) / 2.0);

    RK4Solver<double>::State u_start = { p.u0 };

    auto system = [A](double x, const Vector<double>& v) {
        return Vector<double>{ A * v.data[0] };
    };

    auto exactFunc = [p, A](double x) {
        return Vector<double>{ p.u0 * std::exp(A * (x - p.x0)) };
    };

    RK4Solver<double>::Config config;
    config.initial_h = p.h;
    config.eps = p.eps;
    config.adaptive = p.adaptive;
    config.max_steps = p.max_steps;

    RK4Solver<double>::solve(p.x0, p.b, u_start, system,
                             [&](const RK4Solver<double>::StepInfo& info) {

                                 results.steps.append(info);

                                 double exact = exactFunc(info.x).data[0];
                                 results.exactValues.append(exact);

                                 // Статистика
                                 auto& s = results.stats;
                                 s.n_steps++;
                                 s.total_c1 += info.c1;
                                 s.total_c2 += info.c2;
                                 s.end_x = info.x;

                                 if (std::abs(info.olp) > s.max_olp) s.max_olp = std::abs(info.olp);

                                 if (info.h > s.max_h) { s.max_h = info.h; s.x_at_max_h = info.x; }
                                 if (info.h < s.min_h) { s.min_h = info.h; s.x_at_min_h = info.x; }

                                 double err = std::abs(info.v.data[0] - exact);
                                 if (err > s.max_global_err) { s.max_global_err = err; s.x_at_max_err = info.x; }
                             },
                             config
                             );

    return results;
}

SimulationResults TaskSolver::solveMain(const MainParams& p) {
    SimulationResults results;

    RK4Solver<double>::State u_start = { p.u0, p.v0 };

    auto system = [p](double x, const Vector<double>& st) {
        double u = st.data[0];
        double v = st.data[1];
        double dv = (-p.c * v - p.k * u - p.k_star * std::pow(u, 3)) / p.m;
        return Vector<double>{v, dv};
    };

    RK4Solver<double>::Config config;
    config.initial_h = p.h;
    config.eps = p.eps;
    config.adaptive = p.adaptive;
    config.max_steps = p.max_steps;

    RK4Solver<double>::solve(p.x0, p.b, u_start, system,
                             [&](const RK4Solver<double>::StepInfo& info) {
                                 results.steps.append(info);

                                 // Статистика
                                 auto& s = results.stats;
                                 s.n_steps++;
                                 s.total_c1 += info.c1;
                                 s.total_c2 += info.c2;
                                 s.end_x = info.x;

                                 if (std::abs(info.olp) > s.max_olp) s.max_olp = std::abs(info.olp);
                                 if (info.h > s.max_h) { s.max_h = info.h; s.x_at_max_h = info.x; }
                                 if (info.h < s.min_h) { s.min_h = info.h; s.x_at_min_h = info.x; }
                             },
                             config
                             );

    return results;
}
