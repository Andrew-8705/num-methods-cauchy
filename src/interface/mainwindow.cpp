#include "interface/mainwindow.h"
#include "./ui_mainwindow.h"
#include "TaskSolver.h"
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupTables();

    QList<QCustomPlot*> plots = {ui->plotTest, ui->plotMainU, ui->plotMainV, ui->plotPhase};
    for(auto plot : plots) {
        plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    }

    ui->tabWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupTables() {
    // Тестовая задача
    QStringList headersTest = {
        "i",
        "xᵢ",
        "vᵢ",
        "v₂ᵢ",
        "vᵢ - v₂ᵢ",
        "OLP",
        "hᵢ",
        "C1",
        "C2",
        "uᵢ",
        "|uᵢ - vᵢ|"
    };

    ui->tableTest->setColumnCount(headersTest.size());
    ui->tableTest->setHorizontalHeaderLabels(headersTest);
    ui->tableTest->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableTest->verticalHeader()->setVisible(false);
    ui->tableTest->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Основная задача
    QStringList headersMain = {
        "i",
        "xᵢ",
        "vᵢ",
        "v₂ᵢ",
        "vᵢ - v₂ᵢ",
        "OLP",
        "hᵢ",
        "C1",
        "C2",
    };
    ui->tableMain->setColumnCount(headersMain.size());
    ui->tableMain->setHorizontalHeaderLabels(headersMain);
    ui->tableMain->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableMain->verticalHeader()->setVisible(false);
    ui->tableMain->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

QTableWidgetItem* MainWindow::createCell(double val, int precision) {
    // 'g' - научная нотация, 'f' - обычная нотация
    return new QTableWidgetItem(QString::number(val, 'g', precision));
}

TestParams MainWindow::getTestParams() {
    TestParams p;
    p.N = ui->spinTest_N->value();
    p.u0 = ui->spinTest_U0->value();
    p.x0 = ui->spinTest_X0->value();
    p.b = ui->spinTest_B->value();
    p.h = ui->spinTest_H->value();
    p.eps = ui->spinTest_Eps->value();
    p.adaptive = ui->checkTest_Control->isChecked();
    p.max_steps = ui->spinMaxIter->value();
    return p;
}

MainParams MainWindow::getMainParams() {
    MainParams p;
    p.m = ui->spinMain_M->value();
    p.c = ui->spinMain_C->value();
    p.k = ui->spinMain_K->value();
    p.k_star = ui->spinMain_KStar->value();
    p.u0 = ui->spinMain_U0->value();
    p.v0 = ui->spinMain_V0->value();
    p.x0 = ui->spinMain_X0->value();
    p.b = ui->spinMain_B->value();
    p.h = ui->spinMain_H->value();
    p.eps = ui->spinMain_Eps->value();
    p.adaptive = ui->checkMain_Control->isChecked();
    p.max_steps = ui->spinMaxIter->value();
    return p;
}

void MainWindow::on_btnCalcTest_clicked() {
    TestParams params = getTestParams();

    SimulationResults results = TaskSolver::solveTest(params);

    updateTestTable(results);
    updateTestCharts(results);
    showStats(results, params.b, true);
}

void MainWindow::updateTestTable(const SimulationResults& results) {
    // Отключить отрисовку для скорости
    ui->tableTest->setUpdatesEnabled(false);
    ui->tableTest->setSortingEnabled(false);

    ui->tableTest->setRowCount(results.steps.size());

    for (int i = 0; i < results.steps.size(); ++i) {
        const auto& info = results.steps[i];
        double exact = results.exactValues[i];
        double err = std::abs(info.v.data[0] - exact);

        ui->tableTest->setItem(i, 0, new QTableWidgetItem(QString::number(info.iter)));
        ui->tableTest->setItem(i, 1, createCell(info.x));
        ui->tableTest->setItem(i, 2, createCell(info.v.data[0]));
        ui->tableTest->setItem(i, 3, createCell(info.v2.data[0]));
        ui->tableTest->setItem(i, 4, createCell(info.v.data[0] - info.v2.data[0], 2));
        ui->tableTest->setItem(i, 5, createCell(info.olp, 2));
        ui->tableTest->setItem(i, 6, createCell(info.h));
        ui->tableTest->setItem(i, 7, new QTableWidgetItem(QString::number(info.c1)));
        ui->tableTest->setItem(i, 8, new QTableWidgetItem(QString::number(info.c2)));
        ui->tableTest->setItem(i, 9, createCell(exact));
        ui->tableTest->setItem(i, 10, createCell(err, 2));
    }

    // Включить отрисовку обратно
    ui->tableTest->setUpdatesEnabled(true);
}

void MainWindow::updateTestCharts(const SimulationResults& results) {
    ui->plotTest->clearGraphs();

    QVector<double> x, yAppr, yExact;

    int n = results.steps.size();
    x.reserve(n); yAppr.reserve(n); yExact.reserve(n);

    for(int i=0; i<n; ++i) {
        x.append(results.steps[i].x);
        yAppr.append(results.steps[i].v.data[0]);
        yExact.append(results.exactValues[i]);
    }

    // График 1: RK4
    ui->plotTest->addGraph();
    ui->plotTest->graph(0)->setData(x, yAppr);
    ui->plotTest->graph(0)->setPen(QPen(Qt::blue));
    ui->plotTest->graph(0)->setName("RK4");
    if (n < 500) ui->plotTest->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

    // График 2: Exact
    ui->plotTest->addGraph();
    ui->plotTest->graph(1)->setData(x, yExact);
    ui->plotTest->graph(1)->setPen(QPen(Qt::red, 2, Qt::DashLine));
    ui->plotTest->graph(1)->setName("Exact");

    ui->plotTest->legend->setVisible(true);
    ui->plotTest->rescaleAxes();
    ui->plotTest->replot();
}

void MainWindow::on_btnSolveMain_clicked() {
    MainParams params = getMainParams();

    SimulationResults results = TaskSolver::solveMain(params);

    updateMainTable(results);
    updateMainCharts(results);
    showStats(results, params.b, false);
}


void MainWindow::updateMainTable(const SimulationResults& results) {
    ui->tableMain->setUpdatesEnabled(false);
    ui->tableMain->setSortingEnabled(false);
    ui->tableMain->setRowCount(results.steps.size());

    for (int i = 0; i < results.steps.size(); ++i) {
        const auto& info = results.steps[i];

        ui->tableMain->setItem(i, 0, new QTableWidgetItem(QString::number(info.iter)));
        ui->tableMain->setItem(i, 1, createCell(info.x));
        ui->tableMain->setItem(i, 2, createCell(info.v.data[0]));
        ui->tableMain->setItem(i, 3, createCell(info.v2.data[0]));
        ui->tableMain->setItem(i, 4, createCell(info.v.data[0] - info.v2.data[0], 2));
        ui->tableMain->setItem(i, 5, createCell(info.olp, 2));
        ui->tableMain->setItem(i, 6, createCell(info.h));
        ui->tableMain->setItem(i, 7, new QTableWidgetItem(QString::number(info.c1)));
        ui->tableMain->setItem(i, 8, new QTableWidgetItem(QString::number(info.c2)));
    }
    ui->tableMain->setUpdatesEnabled(true);
}

void MainWindow::updateMainCharts(const SimulationResults& results) {
    // Очистка
    ui->plotMainU->clearGraphs();
    ui->plotMainV->clearGraphs();
    ui->plotPhase->clearGraphs();
    ui->plotPhase->clearPlottables(); // Удаляет и Graphs, и Curves

    int n = results.steps.size();
    QVector<double> t(n), u(n), v(n);

    for(int i=0; i<n; ++i) {
        t[i] = results.steps[i].x;
        u[i] = results.steps[i].v.data[0];
        v[i] = results.steps[i].v.data[1];
    }

    // --- ГРАФИК 1: Смещение u(t) ---
    ui->plotMainU->addGraph();
    ui->plotMainU->graph(0)->setData(t, u);
    ui->plotMainU->xAxis->setLabel("Время t");
    ui->plotMainU->yAxis->setLabel("Смещение u");
    ui->plotMainU->rescaleAxes();
    ui->plotMainU->replot();

    // --- ГРАФИК 2: Скорость v(t) (или u'(x)) ---
    ui->plotMainV->addGraph();
    ui->plotMainV->graph(0)->setData(t, v);
    ui->plotMainV->graph(0)->setPen(QPen(Qt::red));
    ui->plotMainV->xAxis->setLabel("Время t");
    ui->plotMainV->yAxis->setLabel("Скорость u'");
    ui->plotMainV->rescaleAxes();
    ui->plotMainV->replot();

    // Фазовый портрет
    QCPCurve *curve = new QCPCurve(ui->plotPhase->xAxis, ui->plotPhase->yAxis);
    curve->setData(t, u, v);
    curve->setPen(QPen(Qt::darkGreen, 2));
    ui->plotPhase->xAxis->setLabel("Смещение u");
    ui->plotPhase->yAxis->setLabel("Скорость u'");
    ui->plotPhase->rescaleAxes();
    ui->plotPhase->replot();
}

void MainWindow::showStats(const SimulationResults& results, double target_b, bool isTest) {
    const auto& s = results.stats;

    QString html = "<h3>Результаты расчета:</h3>";
    html += "<table border='0' cellpadding='2'>";

    auto row = [](QString name, QString val) {
        return QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(name, val);
    };

    html += row("Итераций (n):", QString::number(s.n_steps));
    html += row("Погрешность границы (b - x<sub>n</sub>):", QString::number(target_b - s.end_x, 'e', 2));
    html += row("Макс. ОЛП:", QString::number(s.max_olp, 'e', 2));
    html += row("Делений шага:", QString::number(s.total_c1));
    html += row("Удвоений шага:", QString::number(s.total_c2));

    html += row("Макс. шаг (h):", QString("%1 (при x=%2)")
                                      .arg(s.max_h, 0, 'g', 4).arg(s.x_at_max_h, 0, 'f', 2));

    html += row("Мин. шаг (h):", QString("%1 (при x=%2)")
                                     .arg(s.min_h, 0, 'g', 4).arg(s.x_at_min_h, 0, 'f', 2));

    if (isTest) {
        html += row("Макс. глоб. ошибка:", QString("%1 (при x=%2)")
                                               .arg(s.max_global_err, 0, 'e', 2).arg(s.x_at_max_err, 0, 'f', 2));
    }

    html += "</table>";

    QLabel* label = isTest ? ui->labelTest_Stats : ui->labelMain_Stats;
    //QLabel* label = ui->labelTest_Stats;
    label->setText(html);
}
