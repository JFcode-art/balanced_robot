
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    serial(nullptr),
    chartTimer(nullptr),
    timeElapsed(0),
    currentAngle(0),
    currentSpeed(0),
    currentPosition(0),
    currentPWM(0)
{
    ui->setupUi(this);
    
    connect(ui->connectBtn, &QPushButton::clicked, this, &MainWindow::onConnectBtnClicked);
    connect(ui->sendSpeedBtn, &QPushButton::clicked, this, &MainWindow::onSendSpeedBtnClicked);
    connect(ui->sendAngleBtn, &QPushButton::clicked, this, &MainWindow::onSendAngleBtnClicked);
    connect(ui->sendPositionBtn, &QPushButton::clicked, this, &MainWindow::onSendPositionBtnClicked);
    connect(ui->stopBtn, &QPushButton::clicked, this, &MainWindow::onStopBtnClicked);
    connect(ui->clearBtn, &QPushButton::clicked, this, &MainWindow::onClearBtnClicked);
    
    updateSerialPortList();
    
    serial = new QSerialPort(this);
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::onSerialDataReceived);
    
    initCharts();
    
    chartTimer = new QTimer(this);
    connect(chartTimer, &QTimer::timeout, this, &MainWindow::updateCharts);
    chartTimer->start(50);
}

MainWindow::~MainWindow()
{
    if (serial && serial->isOpen()) {
        serial->close();
    }
    delete ui;
}

void MainWindow::updateSerialPortList()
{
    ui->portComboBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->portComboBox->addItem(info.portName() + " - " + info.description());
    }
}

void MainWindow::initCharts()
{
    angleSeries = new QLineSeries();
    angleSeries->setName("角度");
    angleChart = new QChart();
    angleChart->addSeries(angleSeries);
    angleChart->setTitle("角度变化曲线");
    angleAxisX = new QValueAxis();
    angleAxisX->setRange(0, 10);
    angleAxisX->setLabelFormat("%.1f");
    angleAxisX->setTitleText("时间(s)");
    angleAxisY = new QValueAxis();
    angleAxisY->setRange(-30, 30);
    angleAxisY->setLabelFormat("%.1f");
    angleAxisY->setTitleText("角度(°)");
    angleChart->addAxis(angleAxisX, Qt::AlignBottom);
    angleChart->addAxis(angleAxisY, Qt::AlignLeft);
    angleSeries->attachAxis(angleAxisX);
    angleSeries->attachAxis(angleAxisY);
    angleChartView = new QChartView(angleChart);
    angleChartView->setRenderHint(QPainter::Antialiasing);
    ui->angleChartLayout->addWidget(angleChartView);
    
    speedSeries = new QLineSeries();
    speedSeries->setName("速度");
    speedChart = new QChart();
    speedChart->addSeries(speedSeries);
    speedChart->setTitle("速度变化曲线");
    speedAxisX = new QValueAxis();
    speedAxisX->setRange(0, 10);
    speedAxisX->setLabelFormat("%.1f");
    speedAxisX->setTitleText("时间(s)");
    speedAxisY = new QValueAxis();
    speedAxisY->setRange(-50, 50);
    speedAxisY->setLabelFormat("%.1f");
    speedAxisY->setTitleText("速度");
    speedChart->addAxis(speedAxisX, Qt::AlignBottom);
    speedChart->addAxis(speedAxisY, Qt::AlignLeft);
    speedSeries->attachAxis(speedAxisX);
    speedSeries->attachAxis(speedAxisY);
    speedChartView = new QChartView(speedChart);
    speedChartView->setRenderHint(QPainter::Antialiasing);
    ui->speedChartLayout->addWidget(speedChartView);
    
    positionSeries = new QLineSeries();
    positionSeries->setName("位置");
    positionChart = new QChart();
    positionChart->addSeries(positionSeries);
    positionChart->setTitle("位置变化曲线");
    positionAxisX = new QValueAxis();
    positionAxisX->setRange(0, 10);
    positionAxisX->setLabelFormat("%.1f");
    positionAxisX->setTitleText("时间(s)");
    positionAxisY = new QValueAxis();
    positionAxisY->setRange(-100, 100);
    positionAxisY->setLabelFormat("%.1f");
    positionAxisY->setTitleText("位置");
    positionChart->addAxis(positionAxisX, Qt::AlignBottom);
    positionChart->addAxis(positionAxisY, Qt::AlignLeft);
    positionSeries->attachAxis(positionAxisX);
    positionSeries->attachAxis(positionAxisY);
    positionChartView = new QChartView(positionChart);
    positionChartView->setRenderHint(QPainter::Antialiasing);
    ui->positionChartLayout->addWidget(positionChartView);
}

void MainWindow::onConnectBtnClicked()
{
    if (serial->isOpen()) {
        serial->close();
        ui->connectBtn->setText("连接");
        ui->statusLabel->setText("状态: 未连接");
        ui->statusLabel->setStyleSheet("color: red");
    } else {
        QString portName = ui->portComboBox->currentText().split(" - ").first();
        QString baudRate = ui->baudComboBox->currentText();
        
        serial->setPortName(portName);
        serial->setBaudRate(baudRate.toInt());
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        
        if (serial->open(QIODevice::ReadWrite)) {
            ui->connectBtn->setText("断开");
            ui->statusLabel->setText("状态: 已连接");
            ui->statusLabel->setStyleSheet("color: green");
        } else {
            QMessageBox::warning(this, "连接失败", "无法打开串口: " + serial->errorString());
        }
    }
}

void MainWindow::sendCommand(uint8_t cmd, const QByteArray& data)
{
    if (!serial->isOpen()) {
        QMessageBox::warning(this, "警告", "请先连接串口");
        return;
    }
    
    QByteArray packet;
    packet.append(0xAA);
    packet.append(cmd);
    packet.append(data);
    packet.append(0x55);
    
    serial->write(packet);
}

void MainWindow::onSendSpeedBtnClicked()
{
    float speed = ui->speedSpinBox->value();
    int16_t speedInt = static_cast<int16_t>(speed * 100);
    QByteArray data;
    data.append((speedInt >> 8) & 0xFF);
    data.append(speedInt & 0xFF);
    sendCommand(0x01, data);
}

void MainWindow::onSendAngleBtnClicked()
{
    float angle = ui->angleSpinBox->value();
    int16_t angleInt = static_cast<int16_t>(angle * 100);
    QByteArray data;
    data.append((angleInt >> 8) & 0xFF);
    data.append(angleInt & 0xFF);
    sendCommand(0x02, data);
}

void MainWindow::onSendPositionBtnClicked()
{
    float position = ui->positionSpinBox->value();
    int16_t posInt = static_cast<int16_t>(position * 10);
    QByteArray data;
    data.append((posInt >> 8) & 0xFF);
    data.append(posInt & 0xFF);
    sendCommand(0x06, data);
}

void MainWindow::onStopBtnClicked()
{
    sendCommand(0x04, QByteArray());
    ui->speedSpinBox->setValue(0);
    ui->angleSpinBox->setValue(0);
    ui->positionSpinBox->setValue(0);
}

void MainWindow::onClearBtnClicked()
{
    angleData.clear();
    speedData.clear();
    positionData.clear();
    timeElapsed = 10.0;
    
    angleSeries->clear();
    speedSeries->clear();
    positionSeries->clear();
    
    angleAxisX->setRange(0, 10);
    speedAxisX->setRange(0, 10);
    positionAxisX->setRange(0, 10);
    
    currentAngle = 0;
    currentSpeed = 0;
    currentPosition = 0;
    currentPWM = 0;
    
    ui->angleValueLabel->setText("0.00°");
    ui->speedValueLabel->setText("0.00");
    ui->positionValueLabel->setText("0.00");
    ui->pwmValueLabel->setText("0");
}

void MainWindow::onSerialDataReceived()
{
    static QByteArray buffer;
    buffer.append(serial->readAll());
    
    if (buffer.size() > 1024) {
        buffer.clear();
        return;
    }
    
    while (buffer.size() >= 9) {
        int startIdx = buffer.indexOf(static_cast<char>(0xAA));
        if (startIdx == -1) {
            buffer.remove(0, buffer.size() - 8);
            return;
        }
        
        if (startIdx > 0) {
            buffer.remove(0, startIdx);
        }
        
        if (buffer.size() >= 9 && buffer[8] == static_cast<char>(0x55)) {
            uint8_t cmd = static_cast<uint8_t>(buffer[1]);
            if (cmd == 0x01) {
                qint8 rawAngle = static_cast<qint8>(buffer[2]);
                qint8 rawSpeed = static_cast<qint8>(buffer[3]);
                qint16 rawPos = static_cast<qint16>(
                    static_cast<uint8_t>(buffer[4]) << 8 | static_cast<uint8_t>(buffer[5]));
                qint16 rawPWM = static_cast<qint16>(
                    static_cast<uint8_t>(buffer[6]) << 8 | static_cast<uint8_t>(buffer[7]));
                
                currentAngle = rawAngle / 10.0f;
                currentSpeed = static_cast<float>(rawSpeed);
                currentPosition = rawPos / 10.0f;
                currentPWM = static_cast<float>(rawPWM);
                
                ui->angleValueLabel->setText(QString::number(currentAngle, 'f', 2) + "°");
                ui->speedValueLabel->setText(QString::number(currentSpeed, 'f', 2));
                ui->positionValueLabel->setText(QString::number(currentPosition, 'f', 2));
                ui->pwmValueLabel->setText(QString::number(static_cast<int>(currentPWM)));
            }
            
            buffer = buffer.mid(9);
        } else {
            break;
        }
    }
}

void MainWindow::updateCharts()
{
    timeElapsed += 0.05;
    
    angleData.append(currentAngle);
    speedData.append(currentSpeed);
    positionData.append(currentPosition);
    
    if (angleData.size() > 200) {
        angleData.removeFirst();
        speedData.removeFirst();
        positionData.removeFirst();
    }
    
    QVector<QPointF> anglePoints(angleData.size());
    QVector<QPointF> speedPoints(speedData.size());
    QVector<QPointF> positionPoints(positionData.size());
    
    double startTime = timeElapsed - (angleData.size() * 0.05);
    for (int i = 0; i < angleData.size(); i++) {
        double x = startTime + i * 0.05;
        anglePoints[i] = QPointF(x, angleData[i]);
        speedPoints[i] = QPointF(x, speedData[i]);
        positionPoints[i] = QPointF(x, positionData[i]);
    }
    
    angleSeries->replace(anglePoints);
    speedSeries->replace(speedPoints);
    positionSeries->replace(positionPoints);
    
    double maxTime = qMax(timeElapsed, 10.0);
    angleAxisX->setRange(maxTime - 10, maxTime);
    speedAxisX->setRange(maxTime - 10, maxTime);
    positionAxisX->setRange(maxTime - 10, maxTime);
}
