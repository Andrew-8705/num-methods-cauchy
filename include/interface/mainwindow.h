#pragma once

#include <QMainWindow>
#include <QTableView>
#include "../math/RK4Solver.hpp"
#include "qcustomplot.h"
#include "interface/ResultsModel.h"

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

    SimulationResults m_testResults;
    SimulationResults m_mainResults;

    ResultsModel* m_testModel;
    ResultsModel* m_mainModel;

    void setupTablesViews();

    TestParams getTestParams();
    MainParams getMainParams();

    void updateTestCharts(const SimulationResults& results);
    void updateMainCharts(const SimulationResults& results);
    void showStats(const SimulationResults& results, double target_b, bool isTest);
};
