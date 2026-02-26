#pragma once

#include <QMainWindow>
#include "../math/RK4Solver.hpp"
#include "qcustomplot.h"
#include "SimulationData.h"

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

    void setupTables();

    void updateTestTable(const SimulationResults& results);
    void updateMainTable(const SimulationResults& results);

    void updateTestCharts(const SimulationResults& results);
    void updateMainCharts(const SimulationResults& results);

    void showStats(const SimulationResults& results, double target_b, bool isTest);

    QTableWidgetItem* createCell(double val, int precision = 6);

    TestParams getTestParams();
    MainParams getMainParams();
};
