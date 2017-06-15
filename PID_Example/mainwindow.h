#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore/QTimer>
#include <QtCharts/QChart>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>

QT_CHARTS_BEGIN_NAMESPACE
class QSplineSeries;
class QValueAxis;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_lineEdit_Kp_textChanged(const QString &arg1);

    void on_lineEdit_Ki_textChanged(const QString &arg1);

    void on_lineEdit_Kd_textChanged(const QString &arg1);

    void on_horizontalSlider_SetPoint_valueChanged(int value);

    void on_horizontalSlider_Interval_valueChanged(int value);

private:
    void update_data_from_ui();
    void updata_data_to_ui();
    void BeginTimer();
    void RestartTimer();
    void PauseTimer();
    void EndTimer();
    void InitChart();
private slots:
    void handle_tmr_PV_Timeout();
    void handle_tmr_Chart_Timeout();
    void handle_tmr_Noise_Timeout();
    void handle_tmr_pid_ctrl_Timeout();
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
private:
    QTimer m_tmrPV;
    QTimer m_tmrChart;
    QTimer m_tmrNoise;
    QTimer m_tmrPID_Ctrl;
private:
    double m_dSetPoint{0.0};
    double m_dPV{0.0};//actual possition (Process Value)
    double m_dError{0.0};//how much SP and PV are diff (SP - PV)
    double m_dIntegral{0.0};//curIntegral + (error * Delta Time)
    double m_dDerivative{0.0};//(error - prev error) / Delta time
    double m_dPreError{0.0};//error from last time (previous Error)
    double m_dKp{0.2};//PID constant multipliers
    double m_dKi{0.01};
    double m_dKd{1.0};
    double m_dDeltaTime{100.0};//delta time, the interval between saples (in milliseconds).
    double m_dOutput{0.0};//the drive amount that effects the PV.
    double m_dNoisePercent{0.02};//amount of the full range to randomly alter the PV
    double m_dNoise{0.0};//random noise added to PV
private:
    QChart* m_chart;
    QChartView* m_chart_view;
    QSplineSeries* m_series_SP;
    QSplineSeries* m_series_PV;
    QSplineSeries* m_series_MV;
    QValueAxis* m_axisX;
    QValueAxis* m_axisY;
    int m_x{0};
    int m_xTemp{0};
    bool m_bTimer_enable{true};
};

#endif // MAINWINDOW_H
