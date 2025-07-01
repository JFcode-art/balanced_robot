
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QChart>
#include <QLineSeries>
#include <QChartView>
#include <QValueAxis>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectBtnClicked();
    void onSendSpeedBtnClicked();
    void onSendAngleBtnClicked();
    void onSendPositionBtnClicked();
    void onStopBtnClicked();
    void onClearBtnClicked();
    void onSerialDataReceived();
    void updateCharts();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    QTimer *chartTimer;
    
    QChart *angleChart;
    QChart *speedChart;
    QChart *positionChart;
    QLineSeries *angleSeries;
    QLineSeries *speedSeries;
    QLineSeries *positionSeries;
    QChartView *angleChartView;
    QChartView *speedChartView;
    QChartView *positionChartView;
    
    QValueAxis *angleAxisX;
    QValueAxis *angleAxisY;
    QValueAxis *speedAxisX;
    QValueAxis *speedAxisY;
    QValueAxis *positionAxisX;
    QValueAxis *positionAxisY;
    
    QList<double> angleData;
    QList<double> speedData;
    QList<double> positionData;
    double timeElapsed;
    
    float currentAngle;
    float currentSpeed;
    float currentPosition;
    float currentPWM;
    
    void initCharts();
    void updateSerialPortList();
    void sendCommand(uint8_t cmd, const QByteArray& data);
};

#endif // MAINWINDOW_H
