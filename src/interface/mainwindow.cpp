#include "interface/mainwindow.h"
#include "./ui_mainwindow.h"
#include "TaskSolver.h"
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupTablesViews();

    QList<QCustomPlot*> plots = {ui->plotTest, ui->plotMainU, ui->plotMainV, ui->plotPhase};
    for(auto plot : plots) {
        plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    }

    ui->tabWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupTablesViews() {
    m_testModel = new ResultsModel(ResultsModel::TestTask, this);
    m_mainModel = new ResultsModel(ResultsModel::MainTask, this);

    ui->tableTest->setModel(m_testModel);
    ui->tableMain->setModel(m_mainModel);

    auto setupView = [](QTableView* view) {
        view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        view->horizontalHeader()->setDefaultSectionSize(100);
        view->verticalHeader()->setVisible(false);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setAlternatingRowColors(true);
    };

    setupView(ui->tableTest);
    setupView(ui->tableMain);
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
    p.max_steps = ui->spinMain_MaxIter->value();
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
    p.max_steps = ui->spinMain_MaxIter->value();
    return p;
}

void MainWindow::on_btnCalcTest_clicked() {
    TestParams params = getTestParams();

    m_testResults = TaskSolver::solveTest(params);
    m_testModel->setResults(&m_testResults);

    updateTestCharts(m_testResults);
    showStats(m_testResults, params.b, true);
}

void MainWindow::on_btnSolveMain_clicked() {
    MainParams params = getMainParams();

    m_mainResults = TaskSolver::solveMain(params);
    m_mainModel->setResults(&m_mainResults);

    updateMainCharts(m_mainResults);
    showStats(m_mainResults, params.b, false);
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

    html += row("Погрешность границы (b - x<sub>n</sub>):", QString::number(target_b - s.end_x, 'f', 10));

    html += row("Макс. ОЛП:", QString::number(s.max_olp, 'f', 10));

    html += row("Делений шага:", QString::number(s.total_c1));
    html += row("Удвоений шага:", QString::number(s.total_c2));

    html += row("Макс. шаг (h):", QString("%1 (при x=%2)")
                                      .arg(s.max_h, 0, 'f', 10)
                                      .arg(s.x_at_max_h, 0, 'f', 10));

    html += row("Мин. шаг (h):", QString("%1 (при x=%2)")
                                     .arg(s.min_h, 0, 'f', 10)
                                     .arg(s.x_at_min_h, 0, 'f', 10));

    if (isTest) {
        html += row("Макс. глоб. ошибка:", QString("%1 (при x=%2)")
                                               .arg(s.max_global_err, 0, 'g', 10)
                                               .arg(s.x_at_max_err, 0, 'f', 10));
    }

    html += "</table>";

    QLabel* label = isTest ? ui->labelTest_Stats : ui->labelMain_Stats;
    label->setText(html);
}
