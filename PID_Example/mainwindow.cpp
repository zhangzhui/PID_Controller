#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore/QTime>
#include <QtCore/QDebug>

#define UNREFERENCED_PARAMETER(P) (P)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qsrand((uint) QTime::currentTime().msec());

    updata_data_to_ui();

    InitChart();

    BeginTimer();
}

MainWindow::~MainWindow()
{
    EndTimer();
    delete ui;
}

void MainWindow::update_data_from_ui()
{
    bool ok = false;
    double kp = ui->lineEdit_Kp->text().toDouble(&ok);
    if (ok) m_dKp = kp;

    ok = false;
    double ki = ui->lineEdit_Ki->text().toDouble(&ok);
    if (ok) m_dKi = ki;

    ok = false;
    double kd = ui->lineEdit_Kd->text().toDouble(&ok);
    if (ok) m_dKd = kd;

    m_dSetPoint = double(ui->horizontalSlider_SetPoint->value());
    m_dNoisePercent = ui->spinBox_noise->value() / 100.0;
    m_dDeltaTime = double(ui->horizontalSlider_Interval->value());
}

void MainWindow::updata_data_to_ui()
{
    ui->lineEdit_Kp->setText(QString("%1").arg(m_dKp));
    ui->lineEdit_Ki->setText(QString("%1").arg(m_dKi));
    ui->lineEdit_Kd->setText(QString("%1").arg(m_dKd));
    ui->horizontalSlider_SetPoint->setValue(int(m_dSetPoint));
    ui->progressBar_PV->setValue(int(m_dPV));
    ui->progressBar_MV->setValue(int(m_dOutput));
    ui->horizontalSlider_Interval->setValue(int(m_dIntegral));
    ui->spinBox_noise->setValue(int(m_dNoisePercent * 100));
    ui->horizontalSlider_Interval->setValue(int(m_dDeltaTime));
}

void MainWindow::on_lineEdit_Kp_textChanged(const QString &arg1)
{
    bool ok = false;
    double kp = arg1.toDouble(&ok);
    if (ok) m_dKp = kp;
}

void MainWindow::on_lineEdit_Ki_textChanged(const QString &arg1)
{
    bool ok = false;
    double kp = arg1.toDouble(&ok);
    if (ok) m_dKi = kp;
}

void MainWindow::on_lineEdit_Kd_textChanged(const QString &arg1)
{
    bool ok = false;
    double kp = arg1.toDouble(&ok);
    if (ok) m_dKd = kp;
}

void MainWindow::on_horizontalSlider_SetPoint_valueChanged(int value)
{
    QString str = QString("SP (Set Point: %1)").arg(value);
    ui->label_6->setText(str);
    m_dSetPoint = double(value);
}

void MainWindow::on_horizontalSlider_Interval_valueChanged(int value)
{
    QString str = QString("Interval ( m : %1)").arg(value);
    ui->label_8->setText(str);
    m_dIntegral = double(value);
    m_tmrPID_Ctrl.setInterval(int(m_dIntegral));
}

void MainWindow::BeginTimer()
{
    QObject::connect(&m_tmrPV, SIGNAL(timeout()), this, SLOT(handle_tmr_PV_Timeout()));
    QObject::connect(&m_tmrChart, SIGNAL(timeout()), this, SLOT(handle_tmr_Chart_Timeout()));
    QObject::connect(&m_tmrNoise, SIGNAL(timeout()), this, SLOT(handle_tmr_Noise_Timeout()));
    QObject::connect(&m_tmrPID_Ctrl, SIGNAL(timeout()), this, SLOT(handle_tmr_pid_ctrl_Timeout()));
    RestartTimer();
}

void MainWindow::RestartTimer()
{
    m_tmrPV.setInterval(17);
    m_tmrPV.start();

    m_tmrNoise.setInterval(61);
    m_tmrNoise.start();

    m_tmrChart.setInterval(37);
    m_tmrChart.start();

    m_tmrPID_Ctrl.setInterval(int(m_dIntegral));
    m_tmrPID_Ctrl.start();
}

void MainWindow::PauseTimer()
{
    EndTimer();
}

void MainWindow::EndTimer()
{
    if (m_tmrPV.isActive()) m_tmrPV.stop();
    if (m_tmrChart.isActive()) m_tmrChart.stop();
    if (m_tmrNoise.isActive()) m_tmrNoise.stop();
    if (m_tmrPID_Ctrl.isActive()) m_tmrPID_Ctrl.stop();
}

void MainWindow::handle_tmr_PV_Timeout()
{
    //This timer updates the process data. it needs to be the fastest interval in the system.
    /* this my version of cruise control.
     * PV = PV + (output * .2) - (PV*.10);
     * The equation contains values for speed, efficiency,
     *  and wind resistance.
       Here 'PV' is the speed of the car.
       'output' is the amount of gas supplied to the engine.
     * (It is only 20% efficient in this example)
       And it looses energy at 10% of the speed. (The faster the
       car moves the more PV will be reduced.)
     * Noise is added randomly if checked, otherwise noise is 0.0
     * (Noise doesn't relate to the cruise control, but can be useful
     *  when modeling other processes.)
    */
    m_dPV = m_dPV + (m_dOutput * 0.20) - (m_dPV * 0.10) + m_dNoise;
    ui->progressBar_PV->setValue(int(m_dPV));
    ui->label_5->setText(QString("PV(Process Value : %1)").arg(m_dPV));
    // change the above equation to fit your aplication
}

void MainWindow::handle_tmr_Chart_Timeout()
{
    m_series_SP->append(m_x, m_dSetPoint);
    m_series_PV->append(m_x, m_dPV);
    m_series_MV->append(m_x, m_dOutput);
    ++m_x;
    ++m_xTemp;

    if(m_x == 1000)
        EndTimer();

//    if (m_xTemp > m_chart->plotArea().width() * 2 / 3)
//    {
//        m_chart->scroll(100, 0);
//        m_xTemp -= 100;
//    }
}

void MainWindow::handle_tmr_Noise_Timeout()
{
    if (ui->checkBox_Noise->isChecked())
    {
        m_dNoisePercent = ui->spinBox_noise->value() / 100.0;
        double dRandom = qrand() % 101 / (double)100;
        m_dNoise = (ui->progressBar_PV->maximum() * m_dNoisePercent) * (dRandom - 0.5);
    }
    else
        m_dNoise = 0;

    // add a positive or negative noise
    // first get the max allowable noise, then multiply by a random value between -0.5 and 0.5
    /*[The noise doesn't really represent what happens to a car
     * in the real world, but it is usefull if you model other processes.
     * This part of the code into its own timer in order to have much more
     * control over the noise frequency.
    */
}

void MainWindow::handle_tmr_pid_ctrl_Timeout()
{
    /*This represents the speed at which electronics could actualy
        sample the process values.. and do any work on them.
     * [most industrial batching processes are SLOW, on the order of minutes.
     *  but were going to deal in times 10ms to 1 second.
     *  Most PLC's have relativly slow data busses, and would sample
     *  temperatures on the order of 100's of milliseconds. So our
     *  starting time interval is 100ms]
     */

    /*
     * Pseudocode from Wikipedia
     *
        previous_error = 0
        integral = 0
     tart:
        error = setpoint - PV(actual_position)
        integral = integral + error*dt
        derivative = (error - previous_error)/dt
        output = Kp*error + Ki*integral + Kd*derivative
        previous_error = error
        wait(dt)
        goto start
     */

    QString labelText;

    // calculate the difference between the desired value and the actual value
    m_dError = m_dSetPoint - m_dPV;
    labelText = QString("%1").arg(m_dError);
    ui->label_Text_Error->setText(labelText);
    // track error over time, scalechartsd to the timer interval
    m_dIntegral = m_dIntegral + (m_dError * m_dDeltaTime);
    labelText = QString("%1").arg(m_dIntegral);
    ui->label_Text_Integral->setText(labelText);
    // determin the amount of change from the last time checked
    m_dDerivative = (m_dError - m_dPreError) / m_dDeltaTime;
    labelText = QString("%1").arg(m_dDerivative);
    ui->label_Text_Derivative->setText(labelText);
    // calculate how much drive the output in order to get to the
    // desired setpoint.
    m_dOutput = (m_dKp * m_dError) + (m_dKi * m_dIntegral) + (m_dKd * m_dDerivative);
    ui->progressBar_MV->setValue(int(m_dOutput));
    ui->label_7->setText(QString("MV (out put : %1)").arg(m_dOutput));
    // remember the error for the next time around.
    m_dPreError = m_dError;
}

void MainWindow::InitChart()
{
    m_chart = new QChart();
    m_series_SP = new QSplineSeries(m_chart);
    m_series_PV = new QSplineSeries(m_chart);
    m_series_MV = new QSplineSeries(m_chart);
    m_axisX = new QValueAxis;
    m_axisY = new QValueAxis;

    QPen red(Qt::red);
    QPen green(Qt::green);
    QPen blue(Qt::blue);

    m_series_SP->setPen(green);
    m_series_PV->setPen(blue);
    m_series_MV->setPen(red);

    m_chart->addSeries(m_series_SP);
    m_chart->addSeries(m_series_PV);
    m_chart->addSeries(m_series_MV);

    m_chart->setTitle("PID controller Example");
    m_chart->setAnimationOptions(QChart::AllAnimations);
    m_chart_view = new QChartView(m_chart);
    m_chart_view->setRenderHint(QPainter::Antialiasing);

    m_chart->createDefaultAxes();
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_chart->axisX()->setRange(0, 1000);
    m_chart->axisY()->setRange(0, 1000);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_chart_view);
    ui->widget_chart->setLayout(layout);
}

void MainWindow::on_pushButton_clicked()
{
//    if (m_bTimer_enable)
//    {
//        ui->pushButton->setText("Start Process");
//        EndTimer();
//    }
//    else
//    {
//        ui->pushButton->setText("Stop Process");
//        RestartTimer();
//    }
//    m_bTimer_enable = !m_bTimer_enable;
}
