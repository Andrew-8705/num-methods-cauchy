#pragma once

#include <QVector>
#include "./math/RK4Solver.hpp"

// Ввод параметров для тестовой задачи
struct TestParams {
    int N;
    double u0;
    double x0;
    double b;
    double h;
    double eps;
    bool adaptive;
    int max_steps;
};

// Ввод параметров для основной задачи
struct MainParams {
    double m, c, k, k_star;
    double u0, v0;
    double x0, b;
    double h;
    double eps;
    bool adaptive;
    int max_steps;
};

// Статистика
struct RunStats {
    int n_steps = 0;
    double max_olp = 0.0;
    int total_c1 = 0;
    int total_c2 = 0;
    double max_h = -1.0;
    double x_at_max_h = 0.0;
    double min_h = 1e9;
    double x_at_min_h = 0.0;
    double max_global_err = 0.0;
    double x_at_max_err = 0.0;
    double end_x = 0.0;
};

// Результаты
struct SimulationResults {
    QVector<RK4Solver<double>::StepInfo> steps;
    QVector<double> exactValues;
    RunStats stats;
};
