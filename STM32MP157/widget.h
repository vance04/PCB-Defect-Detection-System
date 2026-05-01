#ifndef WIDGET_H
#define WIDGET_H

//tab1
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include "inference.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QElapsedTimer>

//tab2
#include <QSerialPort>
#include <QSerialPortInfo>

//tab3
#include <QDir>
#include <QDateTime>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    //tab1
    void processFrame();
    void onInferButtonClicked();    // 修改：改为实时推理开关
    void beepControl();
    void ledControl();
    void on_stopButton_clicked();
    void on_saveButton_clicked();
    void on_checkBox_checkStateChanged(int state);
    void on_autosaveCheckBox_stateChanged(int arg1);

    //tab2
    void readSerialData();
    void updateSensorDisplay(const QString& data);

    void on_addButton_clicked();

    //tab3
    void on_deleteButton_clicked();



private:
    //tab1
    Ui::Widget *ui;

    Inference *inference;
    QTimer *timer;
    cv::VideoCapture capture;
    cv::Mat currentFrame;
    cv::Mat processedFrame;     // 新增：存储处理后的帧（带检测结果）

    QFile beepFile;
    QFile ledFile;
    int beepStep;
    int ledStep;
    QTimer *beepTimer;
    QTimer *ledTimer;
    void updateImageDisplay(QLabel *label, const cv::Mat &mat);
    void drawDetections(cv::Mat &frame, const std::vector<Detection> &detections);
    void setBeepState(bool state);
    void setLedState(bool state);
    void launchProgram(const QString &path, const QStringList &arguments = QStringList());
    bool findAndKillProcess(const QString &processName);
    int currentImageIndex; // tab3中的图片索引
    QElapsedTimer fpsTimer;
    int frameCount;
    int currentFPS;
    QTimer *fpsUpdateTimer;
    bool alarmEnabled;
    void saveCurrentImage();
    bool autoSaveEnabled;

    // 新增：实时推理控制标志
    bool realTimeInferenceEnabled;  // 实时推理开关标志
    QElapsedTimer inferenceTimer;    // 推理耗时计时器
    double inferenceTime;            // 单次推理耗时(ms)

    int frameSkipCounter;            // 帧计数器，用于跳帧
    int inferenceInterval;           // 推理间隔（每N帧推理1次）
    cv::Mat lastInferenceResult;     // 缓存最近一次的推理结果图像
    // 新增：瑕疵检测相关变量
    bool lastDefectState;        // 上次瑕疵检测状态（用于避免重复发送）
    bool defectTriggered;         // 瑕疵是否已触发舵机动作

    //tab2
    QSerialPort *serialPort;
    QTimer *serialReconnectTimer;
    QString receivedDataBuffer;
    void initSerialPort();
    void setupSerialReconnect();
    void parseAndUpdateSensorData(const QString& dataLine);
    // 新增：串口发送指令函数
    void sendSerialCommand(const QString& cmd);

    //tab3
    QString saveDirPath;
    cv::Mat lastSavedImage;
    void loadSavedImages();
    void showPreviewImage(const QString& imagePath);
};
#endif // WIDGET_H
