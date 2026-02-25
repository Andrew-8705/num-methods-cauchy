#include "interface/mainwindow.h"
#include "./ui_mainwindow.h"
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

void MainWindow::on_btnCalcTest_clicked() {
    SimulationResults results = runTestSimulation();

    updateTestTable(results);
    updateTestCharts(results);

    double b = ui->spinTest_B->value();
    showStats(results, b, true);
}

MainWindow::SimulationResults MainWindow::runTestSimulation() {
    SimulationResults results;

    // Чтение параметров
    int N = ui->spinTest_N->value();
    double u0_val = ui->spinTest_U0->value();
    double x0 = ui->spinTest_X0->value();
    double b = ui->spinTest_B->value();

    // Формула: u' = A*u, A = (-1)^N * N/2
    double A = std::pow(-1, N) * (double(N) / 2.0);

    RK4Solver<double>::State u_start = { u0_val };
    auto system = [A](double x, const Vector<double>& v) {
        return Vector<double>{ A * v.data[0] };
    };

    auto exactFunc = [u0_val, A, x0](double x) {
        return Vector<double>{ u0_val * std::exp(A * (x - x0)) };
    };

    RK4Solver<double>::Config config;
    config.initial_h = ui->spinTest_H->value();
    config.eps = ui->spinTest_Eps->value();
    config.adaptive = ui->checkTest_Control->isChecked();
    config.max_steps = ui->spinMaxIter->value();

    // Запуск решеталя
    RK4Solver<double>::solve(x0, b, u_start, system,
                             [&](const RK4Solver<double>::StepInfo& info) {
                                 // Сохраняем шаг
                                 results.steps.append(info);

                                 // Сохраняем точное значение
                                 double exact = exactFunc(info.x).data[0];
                                 results.exactValues.append(exact);

                                 // Обновляем статистику
                                 results.stats.n_steps++;
                                 results.stats.total_c1 += info.c1;
                                 results.stats.total_c2 += info.c2;
                                 results.stats.end_x = info.x;

                                 if (std::abs(info.olp) > results.stats.max_olp)
                                     results.stats.max_olp = std::abs(info.olp);

                                 if (info.h > results.stats.max_h) {
                                     results.stats.max_h = info.h;
                                     results.stats.x_at_max_h = info.x;
                                 }
                                 if (info.h < results.stats.min_h) {
                                     results.stats.min_h = info.h;
                                     results.stats.x_at_min_h = info.x;
                                 }

                                 double err = std::abs(info.v.data[0] - exact);
                                 if (err > results.stats.max_global_err) {
                                     results.stats.max_global_err = err;
                                     results.stats.x_at_max_err = info.x;
                                 }
                             },
                             config
                             );

    return results;
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
    SimulationResults results = runMainSimulation();
    updateMainTable(results);
    updateMainCharts(results);

    double b = ui->spinMain_B->value();
    showStats(results, b, false);
}

MainWindow::SimulationResults MainWindow::runMainSimulation() {
    SimulationResults results;

    double m = ui->spinMain_M->value();
    double c = ui->spinMain_C->value();
    double k = ui->spinMain_K->value();
    double k_star = ui->spinMain_KStar->value();

    RK4Solver<double>::State u_start = { ui->spinMain_U0->value(), ui->spinMain_V0->value() };

    // Система: u' = v
    //          v' = (-c*v - k*u - k*u^3) / m
    auto system = [m, c, k, k_star](double x, const Vector<double>& st) {
        double u = st.data[0];
        double v = st.data[1];
        double dv = (-c * v - k * u - k_star * std::pow(u, 3)) / m;
        return Vector<double>{v, dv};
    };

    RK4Solver<double>::Config config;
    config.initial_h = ui->spinMain_H->value();
    config.eps = ui->spinMain_Eps->value();
    config.adaptive = ui->checkTest_Control->isChecked();
    config.max_steps = ui->spinMaxIter->value();

    RK4Solver<double>::solve(ui->spinMain_X0->value(), ui->spinMain_B->value(), u_start, system,
                             [&](const RK4Solver<double>::StepInfo& info) {
                                 results.steps.append(info);

                                 // Статистика
                                 results.stats.n_steps++;
                                 results.stats.total_c1 += info.c1;
                                 results.stats.total_c2 += info.c2;
                                 results.stats.end_x = info.x;

                                 if (std::abs(info.olp) > results.stats.max_olp)
                                     results.stats.max_olp = std::abs(info.olp);

                                 if (info.h > results.stats.max_h) {
                                     results.stats.max_h = info.h; results.stats.x_at_max_h = info.x;
                                 }
                                 if (info.h < results.stats.min_h) {
                                     results.stats.min_h = info.h; results.stats.x_at_min_h = info.x;
                                 }
                             },
                             config
                             );
    return results;
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
