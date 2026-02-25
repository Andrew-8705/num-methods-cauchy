#pragma once

#include <QMainWindow>
#include "RK4Solver.hpp"
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_btnCalcTest_clicked();
    void on_btnSolveMain_clicked();

private:
    Ui::MainWindow *ui;

    struct RunStats {
        int n_steps = 0;
        double max_olp = 0.0;
        int total_c1 = 0; // деление
        int total_c2 = 0; // удвоение
        double max_h = -1.0;
        double x_at_max_h = 0.0;
        double min_h = 1e9;
        double x_at_min_h = 0.0;
        double max_global_err = 0.0;
        double x_at_max_err = 0.0;
        double end_x = 0.0;
    };

    struct SimulationResults {
        QVector<RK4Solver<double>::StepInfo> steps; // Все шаги
        QVector<double> exactValues;                // Точные значения (для тестовой)
        RunStats stats;                             // Статистика
    };

    SimulationResults runTestSimulation();
    SimulationResults runMainSimulation();

    void setupTables();

    void updateTestTable(const SimulationResults& results);
    void updateMainTable(const SimulationResults& results);

    void updateTestCharts(const SimulationResults& results);
    void updateMainCharts(const SimulationResults& results);

    void showStats(const SimulationResults& results, double target_b, bool isTest);

    QTableWidgetItem* createCell(double val, int precision = 6);
};
