#include "widget.h"
#include "ui_widget.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>
#include <QPixmap>
#include <QtSerialPort>
#include <QSerialPortInfo>
#include <map>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , currentImageIndex(1)
    , frameCount(0)
    , currentFPS(0)
    , alarmEnabled(false)
    , autoSaveEnabled(false)
    , serialPort(nullptr)
    , serialReconnectTimer(nullptr)
    , realTimeInferenceEnabled(false)  // 新增：初始化实时推理为关闭状态
    , inferenceTime(0.0)
    , frameSkipCounter(0)              // 初始化帧计数器为0
    , inferenceInterval(8)             // 每4帧推理1次（可根据性能调整）
    , lastDefectState(false)      // 新增：初始化瑕疵状态
    , defectTriggered(false)       // 新增：初始化触发标志
{
    ui->setupUi(this);

    //==============================================================================
    //================================tab_1=========================================
    //==============================================================================

    ui->verticalLayout->setAlignment(Qt::AlignCenter);    
    ui->videoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);    

    connect(ui->checkBox, &QCheckBox::stateChanged, this, &Widget::on_checkBox_checkStateChanged);
    //蜂鸣器
    beepFile.setFileName("/sys/devices/platform/leds/leds/beep/brightness");
    beepStep = 0;
    beepTimer = new QTimer(this);
    connect(beepTimer, &QTimer::timeout, this, &Widget::beepControl);
    //led闪烁
    ledFile.setFileName("/sys/devices/platform/leds/leds/sys-led/brightness");
    ledStep = 0;
    ledTimer = new QTimer(this);
    connect(ledTimer, &QTimer::timeout, this, &Widget::ledControl);

    inference = new Inference("/home/root/best_quantized.onnx", cv::Size(320, 320));
    for (int i = 0; i < 3; i++) {
        capture.open(i);
        if (capture.isOpened()) {
            qDebug() << "成功打开摄像头索引: " << i;
            break;
        } else {
            qDebug() << "无法打开摄像头索引: " << i;
        }
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Widget::processFrame);
    timer->start(33);

    // 连接inferButton为开关按钮
    connect(ui->inferButton, &QPushButton::clicked, this, &Widget::onInferButtonClicked);
    ui->inferButton->setText("开启实时推理");  // 修改按钮文字
    ui->statusLabel->setText("摄像头已启动 | 实时推理: 关闭");    

    fpsTimer.start();
    fpsUpdateTimer = new QTimer(this);
    connect(fpsUpdateTimer, &QTimer::timeout, this, [this]() {
        if (fpsTimer.elapsed() > 0) {
            currentFPS = frameCount * 1000 / fpsTimer.elapsed();
        }
        frameCount = 0;
        fpsTimer.restart();
    });
    fpsUpdateTimer->start(1000);

    ui->autosaveCheckBox->setChecked(false);
    connect(ui->autosaveCheckBox, &QCheckBox::stateChanged,
            this, &Widget::on_autosaveCheckBox_stateChanged);

    //==============================================================================
    //================================tab_2 标题“物料界面”===========================
    //==============================================================================

    // // 设置label_3的样式，使其能够显示多行文本
    // ui->label_3->setWordWrap(true);
    // ui->label_3->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    // ui->label_3->setStyleSheet("QLabel { font: 14pt 'Ubuntu'; background-color: rgba(238, 238, 236, 0.8); padding: 10px; }");
    // ui->label_3->setText("等待传感器数据...");

    // 初始化串口
    initSerialPort();

    // 确保显示第一个标签页
    ui->tabWidget->setCurrentIndex(0);

    //==============================================================================
    //================================tab_3=========================================
    //==============================================================================
    saveDirPath = "/home/root/saveimages";
    QDir saveDir(saveDirPath);
    if (!saveDir.exists()) {
        saveDir.mkpath(".");
    }
    loadSavedImages();

    connect(ui->imageListWidget, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
        QString imagePath = item->data(Qt::UserRole).toString();
        showPreviewImage(imagePath);
    });
}

Widget::~Widget()
{
    delete ui;
    delete inference;
    delete timer;
    delete beepTimer;
    delete ledTimer;
    delete fpsUpdateTimer;
    capture.release();
    setBeepState(false);
    setLedState(false);
}

//====================================================================================================================
//=========================================functions-yolo=============================================================
void Widget::processFrame()
{
    capture >> currentFrame;
    if (currentFrame.empty()) return;

    frameCount++;

    cv::Mat displayFrame;

    // 修改：非实时推理模式：只显示原始视频流
    if (!realTimeInferenceEnabled) {
        displayFrame = currentFrame.clone();
        cv::cvtColor(displayFrame, displayFrame, cv::COLOR_BGR2RGB);

        // 显示帧率和状态
        std::string fpsText = "FPS: " + std::to_string(currentFPS);
        cv::putText(displayFrame, fpsText, cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

        std::string inferStatus = "RealTime: OFF";
        cv::putText(displayFrame, inferStatus, cv::Point(displayFrame.cols - 150, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);

        updateImageDisplay(ui->videoLabel, displayFrame);
        return;
    }

    // ========== 实时推理模式（带跳帧处理）==========
    frameSkipCounter++;

    // 判断是否需要执行推理（跳帧处理）
    bool shouldInfer = (frameSkipCounter >= inferenceInterval);

    if (shouldInfer) {
        // 重置帧计数器
        frameSkipCounter = 0;

        // 执行推理
        inferenceTimer.start();
        std::vector<Detection> detections = inference->runInference(currentFrame);
        inferenceTime = inferenceTimer.elapsed();

        // 创建带检测结果的图像
        cv::Mat resultFrame = currentFrame.clone();
        drawDetections(resultFrame, detections);

        // 缓存本次推理结果
        lastInferenceResult = resultFrame.clone();

        // ========== 瑕疵检测串口控制舵机 ==========
        bool hasDefect = !detections.empty();

        // 当检测到瑕疵且之前没有触发时，发送指令给STM32
        if (hasDefect && !defectTriggered) {
            defectTriggered = true;

            // 统计本次瑕疵的种类和数量
            std::map<std::string, int> classCount;
            for (const auto &det : detections) {
                classCount[det.className]++;
            }
            QString detailStr;
            for (const auto &pair : classCount) {
                if (!detailStr.isEmpty()) detailStr += ",";
                detailStr += QString::fromStdString(pair.first) + ":" + QString::number(pair.second);
            }
            // 发送完整指令：D1,class1:count,class2:count...
            QString fullCmd = "D1," + detailStr;
            sendSerialCommand(fullCmd);
            qDebug() << "检测到瑕疵！已发送舵机动作指令给STM32，详情：" << detailStr;           
        }
        // 当没有瑕疵时，重置触发标志（允许下次瑕疵再次触发）
        else if (!hasDefect && defectTriggered) {
            defectTriggered = false;
            // 可选：发送无瑕疵指令
            // sendSerialCommand("D0");
        }

        // 保存检测结果用于手动保存
        cv::Mat resultFrameRGB;
        cv::cvtColor(resultFrame, resultFrameRGB, cv::COLOR_BGR2RGB);
        lastSavedImage = resultFrameRGB.clone();

        // 更新状态显示
        if (detections.empty()) {
            ui->statusLabel->setText(QString("实时推理: 开启 | 帧率: %1 FPS | 推理: %2ms | 未检测到瑕疵")
                                         .arg(currentFPS).arg(inferenceTime));
        } else {
            ui->statusLabel->setText(QString("实时推理: 开启 | 帧率: %1 FPS | 推理: %2ms | 检测到 %3 个瑕疵")
                                         .arg(currentFPS).arg(inferenceTime).arg(detections.size()));

            // 触发报警（如果启用）
            if (alarmEnabled) {
                beepStep = 0;
                beepTimer->start(200);
                ledStep = 0;
                setLedState(true);
                ledTimer->start(200);
            }

            // 自动保存（如果启用）
            if (autoSaveEnabled) {
                saveCurrentImage();
            }
        }

        // 使用推理结果作为显示图像
        cv::cvtColor(resultFrame, displayFrame, cv::COLOR_BGR2RGB);

        // 在图像上显示推理耗时和跳帧信息
        std::string inferTimeText = "Infer: " + std::to_string(inferenceTime) + "ms (Skip: " + std::to_string(inferenceInterval-1) + " frames)";
        cv::putText(displayFrame, inferTimeText, cv::Point(10, 60),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 1.5);

    } else {
        // 跳过的帧：使用最近一次的推理结果叠加显示
        if (!lastInferenceResult.empty()) {
            // 使用缓存的推理结果作为当前显示帧（保持检测框显示）
            displayFrame = lastInferenceResult.clone();
            cv::cvtColor(displayFrame, displayFrame, cv::COLOR_BGR2RGB);

            // 添加提示文字表明这是复用帧
            std::string skipText = "Frame Reuse (Infer every " + std::to_string(inferenceInterval) + " frames)";
            cv::putText(displayFrame, skipText, cv::Point(10, 90),
                        cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(255, 200, 100), 1);
        } else {
            // 首次运行，还没有推理结果
            displayFrame = currentFrame.clone();
            cv::cvtColor(displayFrame, displayFrame, cv::COLOR_BGR2RGB);
        }
    }

    // 显示帧率
    std::string fpsText = "FPS: " + std::to_string(currentFPS);
    cv::putText(displayFrame, fpsText, cv::Point(10, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

    // 显示实时推理状态
    std::string inferStatus = "RealTime: ON";
    cv::putText(displayFrame, inferStatus, cv::Point(displayFrame.cols - 150, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);

    updateImageDisplay(ui->videoLabel, displayFrame);
}

void Widget::onInferButtonClicked()
{
    realTimeInferenceEnabled = !realTimeInferenceEnabled;  // 切换状态

    if (realTimeInferenceEnabled) {
        ui->inferButton->setText("关闭实时推理");
        ui->statusLabel->setText(QString("实时推理已开启 | 帧率: %1 FPS").arg(currentFPS));
        qDebug() << "实时推理已开启";

        // 清空afterLabel的提示文字
        // ui->afterLabel->setText("");
    } else {
        ui->inferButton->setText("开启实时推理");
        ui->statusLabel->setText("摄像头已启动 | 实时推理: 关闭");
        qDebug() << "实时推理已关闭";

        // 清空afterLabel显示
        // ui->afterLabel->clear();
        // ui->afterLabel->setText("实时推理已关闭\n点击'开启实时推理'按钮");
    }
}

void Widget::saveCurrentImage()
{
    if (lastSavedImage.empty()) {
        qDebug() << "没有图像可供保存";
        return;
    }
    QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz") + ".png";
    QString fullPath = saveDirPath + "/" + fileName;

    if (cv::imwrite(fullPath.toStdString(), lastSavedImage)) {
        qDebug() << "图片已自动保存至:" << fullPath;
        loadSavedImages();
    } else {
        qDebug() << "自动保存失败:" << fullPath;
    }
}

void Widget::beepControl()
{
    switch (beepStep) {
    case 0:
        setBeepState(true);
        beepStep++;
        break;
    case 1:
        setBeepState(false);
        beepStep++;
        break;
    case 2:
        setBeepState(true);
        beepStep++;
        break;
    case 3:
        setBeepState(false);
        beepTimer->stop();
        break;
    }
}

void Widget::setBeepState(bool state)
{
    if (!beepFile.exists()) return;
    if (beepFile.open(QIODevice::WriteOnly)) {
        beepFile.write(state ? "1" : "0");
        beepFile.close();
    }
}

void Widget::ledControl()
{
    if (ledStep < 10) {
        setLedState(ledStep % 2 == 0);
        ledStep++;
    } else {
        setLedState(false);
        ledTimer->stop();
    }
}

void Widget::setLedState(bool state)
{
    if (!ledFile.exists()) return;
    if (ledFile.open(QIODevice::WriteOnly)) {
        ledFile.write(state ? "1" : "0");
        ledFile.close();
    }
}

void Widget::drawDetections(cv::Mat &frame, const std::vector<Detection> &detections)
{
    for (const auto &detection : detections) {
        cv::Rect box = detection.box;
        cv::rectangle(frame, box, detection.color, 2);

        std::string label = detection.className + " " + std::to_string(detection.confidence).substr(0, 4);
        int baseline;
        cv::Size textSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
        cv::rectangle(frame,
                      cv::Point(box.x, box.y - textSize.height - 5),
                      cv::Point(box.x + textSize.width, box.y),
                      detection.color, cv::FILLED);
        cv::putText(frame, label,
                    cv::Point(box.x, box.y - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    }
}

void Widget::updateImageDisplay(QLabel *label, const cv::Mat &mat)
{
    QImage img(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(img);
    pixmap = pixmap.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    label->setPixmap(pixmap);
}

void Widget::on_stopButton_clicked()
{
    this->close();
}

void Widget::loadSavedImages()
{
    ui->imageListWidget->clear();
    QDir saveDir(saveDirPath);
    if (!saveDir.exists()) return;

    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg";
    QFileInfoList fileList = saveDir.entryInfoList(filters, QDir::Files, QDir::Time | QDir::Reversed);

    for (const QFileInfo &fileInfo : fileList) {
        QString filePath = fileInfo.absoluteFilePath();
        QListWidgetItem *item = new QListWidgetItem();

        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            QPixmap thumb = pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            item->setIcon(QIcon(thumb));
        }

        item->setText(fileInfo.fileName());
        item->setData(Qt::UserRole, filePath);
        item->setSizeHint(QSize(120, 120));
        ui->imageListWidget->addItem(item);
    }
}

void Widget::showPreviewImage(const QString& imagePath)
{
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        pixmap = pixmap.scaled(ui->previewLabel->size(),
                               Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
        ui->previewLabel->setPixmap(pixmap);
        ui->previewLabel->setAlignment(Qt::AlignCenter);
    } else {
        ui->previewLabel->setText("无法加载图片");
    }
}

void Widget::on_saveButton_clicked()
{
    if (lastSavedImage.empty()) {
        qDebug() << "No image to save";
        return;
    }
    QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz") + ".png";
    QString fullPath = saveDirPath + "/" + fileName;

    if (cv::imwrite(fullPath.toStdString(), lastSavedImage)) {
        qDebug() << "Image saved to:" << fullPath;
        loadSavedImages();

        for (int i = 0; i < ui->imageListWidget->count(); i++) {
            QListWidgetItem *item = ui->imageListWidget->item(i);
            if (item->data(Qt::UserRole).toString() == fullPath) {
                ui->imageListWidget->setCurrentItem(item);
                showPreviewImage(fullPath);
                break;
            }
        }
    } else {
        qDebug() << "Failed to save image to:" << fullPath;
    }
}

void Widget::on_checkBox_checkStateChanged(int state)
{
    alarmEnabled = (state == Qt::Checked);
    if (alarmEnabled) {
        ui->statusLabel->setText("报警功能已启用");
    } else {
        ui->statusLabel->setText("报警功能已禁用");
        setBeepState(false);
        setLedState(false);
        if (beepTimer->isActive()) beepTimer->stop();
        if (ledTimer->isActive()) ledTimer->stop();
    }
}

void Widget::on_autosaveCheckBox_stateChanged(int state)
{
    autoSaveEnabled = (state == Qt::Checked);
    if (autoSaveEnabled) {
        ui->statusLabel->setText("自动保存已启用");
        qDebug() << "自动保存功能已启用";
    } else {
        ui->statusLabel->setText("自动保存已禁用");
        qDebug() << "自动保存功能已禁用";
    }
}
//=========================================tab2 串口相关函数（修正版）=========================================
void Widget::initSerialPort()
{
    // 1. 先释放原有串口对象
    if (serialPort) {
        if (serialPort->isOpen()) serialPort->close();
        delete serialPort;
    }
    serialPort = new QSerialPort(this);

    // 2. 配置目标串口：STM32MP157的USART3（PD9 RX）对应的设备文件
    // 优先匹配ttySTM3（内核驱动命名），其次ttyS3（通用串口命名）
    QString targetPort = "";
    QStringList targetPorts = {"/dev/ttySTM3", "/dev/ttyS3"};
    for (const QString &port : targetPorts) {
        if (QFile::exists(port)) {
            targetPort = port;
            qDebug() << "找到USART3串口设备: " << port;
            break;
        }
    }

    if (targetPort.isEmpty()) {
        qDebug() << "未找到USART3串口设备！请检查：";
        qDebug() << "  - 内核是否启用USART3驱动";
        qDebug() << "  - PD9引脚是否配置为USART3_RX功能";
        qDebug() << "  - 设备文件权限（需chmod 666 /dev/ttySTM3）";
        // ui->label_3->setText("❌ 未找到USART3串口\n"
        //                      "预期设备: /dev/ttySTM3 或 /dev/ttyS3\n"
        //                      "请检查硬件配置和驱动");
        setupSerialReconnect(); // 启动重连（适配后续热插拔/配置）
        return;
    }

    // 3. 配置串口参数（必须与STM32F103的配置完全一致！）
    serialPort->setPortName(targetPort);
    serialPort->setBaudRate(QSerialPort::Baud9600);  // 需与F103的串口波特率匹配
    serialPort->setDataBits(QSerialPort::Data8);     // 8位数据位
    serialPort->setParity(QSerialPort::NoParity);    // 无校验位
    serialPort->setStopBits(QSerialPort::OneStop);   // 1位停止位
    serialPort->setFlowControl(QSerialPort::NoFlowControl); // 无流控

    // 4. 打开串口（处理权限/占用问题）
    if (serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "✅ USART3串口打开成功: " << targetPort;
        connect(serialPort, &QSerialPort::readyRead, this, &Widget::readSerialData);
        //ui->label_3->setText("✅ 串口已连接（USART3）\n等待STM32F103数据...");

        // 清除重连定时器（如果存在）
        if (serialReconnectTimer) {
            serialReconnectTimer->stop();
            delete serialReconnectTimer;
            serialReconnectTimer = nullptr;
        }
    } else {
        QString errorMsg = serialPort->errorString();
        qDebug() << "❌ 串口打开失败: " << errorMsg;
        QString tip = "";
        if (errorMsg.contains("Permission denied")) {
            tip = "\n⚠️  权限不足！执行：sudo chmod 666 " + targetPort;
        } else if (errorMsg.contains("Device or resource busy")) {
            tip = "\n⚠️  串口被占用！检查是否有其他程序使用" + targetPort;
        }
        //ui->label_3->setText("❌ 串口打开失败\n" + errorMsg + tip);
        setupSerialReconnect();
    }
}

void Widget::setupSerialReconnect()
{
    // 释放原有定时器
    if (serialReconnectTimer) {
        serialReconnectTimer->stop();
        delete serialReconnectTimer;
    }

    // 重连逻辑：仅查找USART3对应的设备文件（ttySTM3/ttyS3）
    serialReconnectTimer = new QTimer(this);
    connect(serialReconnectTimer, &QTimer::timeout, [this]() {
        if (!serialPort || serialPort->isOpen()) return;

        qDebug() << "🔄 尝试重新连接USART3串口...";
        QStringList targetPorts = {"/dev/ttySTM3", "/dev/ttyS3"};
        for (const QString &port : targetPorts) {
            if (QFile::exists(port)) {
                serialPort->setPortName(port);
                if (serialPort->open(QIODevice::ReadOnly)) {
                    qDebug() << "✅ 串口重新连接成功: " << port;
                    connect(serialPort, &QSerialPort::readyRead, this, &Widget::readSerialData);
                    //ui->label_3->setText("✅ 串口已重新连接（USART3）\n等待数据...");
                    serialReconnectTimer->stop();
                    break;
                }
            }
        }
    });
    serialReconnectTimer->start(5000); // 每5秒重连一次
}

// 其余函数（readSerialData/parseAndUpdateSensorData）无需修改，保留原有逻辑


void Widget::readSerialData()
{
    if (!serialPort || !serialPort->isOpen()) return;

    // 读取所有可用数据
    QByteArray data = serialPort->readAll();
    receivedDataBuffer += QString::fromUtf8(data);

    // 按行处理数据
    QStringList lines = receivedDataBuffer.split('\n');

    // 处理完整的行
    for (int i = 0; i < lines.size() - 1; ++i) {
        QString line = lines[i].trimmed();
        if (!line.isEmpty()) {
            parseAndUpdateSensorData(line);
        }
    }

    // 保留最后不完整的行
    receivedDataBuffer = lines.last();
}

void Widget::parseAndUpdateSensorData(const QString& dataLine)
{
    // 解析CSV格式数据: 湿度,光敏值,热值，转速，角度
    // 格式示例: "60,850,26.5，0，0"
    QStringList values = dataLine.split(',');

    QString displayText;
    displayText += "═══════════════════════════\n";
    displayText += "     STM32 传感器数据\n";
    displayText += "═══════════════════════════\n\n";

    if (values.size() >= 5) {
        // 解析成功
        QString humi = values[0];
        QString light = values[1];
        QString thermalTemp = values[2];
        QString speed = values[3];
        QString angle = values[4];


        displayText += "   湿度传感器\n";
        displayText += "   湿度: " + humi + " %RH\n\n";

        displayText += "   光敏传感器\n";
        displayText += "   光照强度: " + light + " LUX\n\n";

        displayText += "   热敏传感器\n";
        displayText += "   热值: " + thermalTemp + " °C\n";

        displayText += "   电机\n";
        displayText += "   转速: " + speed + " r/s\n";

        displayText += "   舵机\n";
        displayText += "   角度: " + angle + " °\n";

        displayText += "═══════════════════════════\n";
        displayText += "更新时间: " + QDateTime::currentDateTime().toString("hh:mm:ss");

        qDebug() << "传感器数据 " <<  "湿度:" << humi
                 << "光敏:" << light << "热值:" << thermalTemp<< "转速:" << speed<< "角度:" << angle;
    } else {
        // 如果数据格式不符合预期，直接显示原始数据
        displayText += "原始数据: " + dataLine + "\n\n";
        displayText += "═══════════════════════════\n";
        displayText += "期望格式: 湿度,光敏,热值，转速，角度\n";
        displayText += "示例: 60,850,26.5，0，0";

        qDebug() << "收到数据:" << dataLine;
    }

    //ui->label_3->setText(displayText);
}

void Widget::updateSensorDisplay(const QString& data)
{
    // 这个函数保留用于兼容性，实际使用parseAndUpdateSensorData
    parseAndUpdateSensorData(data);
}


// widget.cpp 新增函数：向STM32F103发送串口指令
void Widget::sendSerialCommand(const QString& cmd)
{
    if (!serialPort || !serialPort->isOpen()) {
        QMessageBox::warning(this, "错误", "串口未连接，无法发送指令！");
        qDebug() << "串口未打开，发送指令失败";
        return;
    }

    // 封装指令格式（匹配STM32F103的串口解析规则：@指令\r\n）
    QString fullCmd;
    // 瑕疵检测指令（格式："D1" 或 "D0"）使用特殊前缀
    if (cmd.startsWith("D")) {
        // 瑕疵指令格式：@D1\r\n
        fullCmd = QString("@%1\r\n").arg(cmd);
    } else {
        // 原有按键指令格式：@1\r\n
        fullCmd = QString("@%1\r\n").arg(cmd);
    }
    QByteArray sendData = fullCmd.toUtf8();

    // 发送数据
    qint64 bytesWritten = serialPort->write(sendData);
    if (bytesWritten == -1) {
        QMessageBox::warning(this, "错误", "指令发送失败：" + serialPort->errorString());
        qDebug() << "串口发送失败：" << serialPort->errorString();
    } else {
        qDebug() << "成功发送指令：" << fullCmd.trimmed();
        //ui->label_3->append("\n📤 发送指令：" + fullCmd.trimmed());
    }
}

void Widget::on_addButton_clicked()
{
    // 发送指令"1"，触发STM32F103的KeyNum=1
    sendSerialCommand("1");

    // 界面反馈
    ui->statusLabel->setText("已发送指令：电机自增");
    //ui->label_3->append("\n✅ 已触发电机/舵机自增操作");
}
//=========================================tab_3========================
void Widget::on_deleteButton_clicked()
{
    QListWidgetItem *currentItem = ui->imageListWidget->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "提示", "请先选择要删除的图片");
        return;
    }

    QString imagePath = currentItem->data(Qt::UserRole).toString();
    QFile file(imagePath);
    if (file.remove()) {
        qDebug() << "已删除图片:" << imagePath;
        int currentRow = ui->imageListWidget->row(currentItem);
        delete currentItem;

        int newCount = ui->imageListWidget->count();
        if (newCount > 0) {
            int newRow = currentRow;
            if (newRow >= newCount) newRow = newCount - 1;
            QListWidgetItem *newItem = ui->imageListWidget->item(newRow);
            ui->imageListWidget->setCurrentItem(newItem);
            QString newImagePath = newItem->data(Qt::UserRole).toString();
            showPreviewImage(newImagePath);
        } else {
            ui->previewLabel->clear();
            ui->previewLabel->setText("无图片");
        }
    } else {
        QMessageBox::warning(this, "错误", "删除图片失败: " + file.errorString());
    }
}

