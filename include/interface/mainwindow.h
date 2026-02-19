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

    void on_btnSolveTest_clicked();

    void on_btnSolveMain_clicked();

private:
    Ui::MainWindow *ui;
};
