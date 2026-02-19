#include "interface/mainwindow.h"
#include "./ui_mainwindow.h"
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ui->lineEdit_Start->setText("0.0");
    // ui->lineEdit_End->setText("2.0");
    // ui->lineEdit_Steps->setText("20");
    // ui->lineEdit_Y0->setText("1.0");

    ui->tabWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_btnSolveTest_clicked()
{
    ui->plotTest->clearGraphs();

    // Параметры тестовой задачи: u' = -u, u(0) = 10
    // Точное решение: u(x) = 10 * exp(-x)
    double u0_val = 10.0;

    // Векторы для графиков
    QVector<double> xData, yApprox, yExact;

    // Система для RK4
    RK4Solver<double>::State u0 = {u0_val};
    auto system = [](double x, const Vector<double>& v) {
        return Vector<double>{ -v.data[0] }; // u' = -u
    };

    // Функция точного решения
    auto exactFunc = [u0_val](double x) {
        return Vector<double>{ u0_val * std::exp(-x) };
    };

    RK4Solver<double>::Config config;
    config.initial_h = 0.1;

    // Решаем
    RK4Solver<double>::solve(0.0, 5.0, u0, system,
                             [&](const RK4Solver<double>::StepInfo& info) {
                                 xData.append(info.x);
                                 yApprox.append(info.v.data[0]);

                                 // Cчитаем точку точного решения
                                 yExact.append(exactFunc(info.x).data[0]);
                             },
                             config
                             );

    // --- ОТРИСОВКА ---

    // График 1: Приближенное решение (Синие точки/линия)
    ui->plotTest->addGraph();
    ui->plotTest->graph(0)->setData(xData, yApprox);
    ui->plotTest->graph(0)->setPen(QPen(Qt::blue));
    ui->plotTest->graph(0)->setName("Приближенное (RK4)");
    // Маркеры точек, чтобы видеть шаги
    ui->plotTest->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

    // График 2: Точное решение (Красный пунктир)
    ui->plotTest->addGraph();
    ui->plotTest->graph(1)->setData(xData, yExact);
    QPen exactPen(Qt::red);
    exactPen.setStyle(Qt::DashLine); // Пунктир
    exactPen.setWidth(2);
    ui->plotTest->graph(1)->setPen(exactPen);
    ui->plotTest->graph(1)->setName("Точное");

    // Легенда
    ui->plotTest->legend->setVisible(true);
    ui->plotTest->xAxis->setLabel("x");
    ui->plotTest->yAxis->setLabel("u(x)");
    ui->plotTest->rescaleAxes();
    ui->plotTest->replot();

    ui->plotTest->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}


void MainWindow::on_btnSolveMain_clicked()
{
    // Очищаем все графики
    ui->plotMainU->clearGraphs();
    ui->plotMainV->clearGraphs();
    ui->plotPhase->clearGraphs();
    ui->plotPhase->clearPlottables(); // Удаляет и Graphs, и Curves

    // Параметры
    double m = 1.0;
    double k = 2.0;
    double c = 0.15;
    double k_star = 2.0; // Нелинейность

    QVector<double> tData, uData, vData;

    RK4Solver<double>::State u0 = {10.0, 0.0}; // u=10, v=0

    // Система: u' = v
    //          v' = (-c*v - k*u - k*u^3) / m
    auto system = [m, k, c, k_star](double x, const Vector<double>& st) {
        double u = st.data[0];
        double v = st.data[1];
        double dv = (-c * v - k * u - k_star * std::pow(u, 3)) / m;
        return Vector<double>{v, dv};
    };

    RK4Solver<double>::Config config;
    config.initial_h = 0.01;

    RK4Solver<double>::solve(0.0, 15.0, u0, system,
                             [&](const RK4Solver<double>::StepInfo& info) {
                                 tData.append(info.x);
                                 uData.append(info.v.data[0]); // Смещение
                                 vData.append(info.v.data[1]); // Скорость
                             },
                             config
                             );

    // --- ГРАФИК 1: Смещение u(t) ---
    ui->plotMainU->addGraph();
    ui->plotMainU->graph(0)->setData(tData, uData);
    ui->plotMainU->xAxis->setLabel("Время t");
    ui->plotMainU->yAxis->setLabel("Смещение u");
    ui->plotMainU->rescaleAxes();
    ui->plotMainU->replot();

    // --- ГРАФИК 2: Скорость v(t) (или u'(x)) ---
    ui->plotMainV->addGraph();
    ui->plotMainV->graph(0)->setData(tData, vData);
    ui->plotMainV->graph(0)->setPen(QPen(Qt::red)); // Красный цвет
    ui->plotMainV->xAxis->setLabel("Время t");
    ui->plotMainV->yAxis->setLabel("Скорость u'");
    ui->plotMainV->rescaleAxes();
    ui->plotMainV->replot();

    // --- ГРАФИК 3: ФАЗОВЫЙ ПОРТРЕТ (v от u) ---
    // Здесь ось X - это смещение, ось Y - это скорость.
    QCPCurve *phaseCurve = new QCPCurve(ui->plotPhase->xAxis, ui->plotPhase->yAxis);

    // Данные передаются как (t, key, value). t здесь просто индекс порядка точек.
    // key = uData (ось X), value = vData (ось Y)
    phaseCurve->setData(tData, uData, vData);

    phaseCurve->setPen(QPen(Qt::darkGreen, 2));
    ui->plotPhase->xAxis->setLabel("Смещение u");
    ui->plotPhase->yAxis->setLabel("Скорость u'");
    ui->plotPhase->rescaleAxes();
    ui->plotPhase->replot();

    // Включаем зум везде
    ui->plotMainU->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->plotMainV->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->plotPhase->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

