QT       += core gui serialport
QT += widgets webenginewidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Opencv4.11.0-arm32
INCLUDEPATH += /home/vance/mine/2019/opencv-4.11.0/build/install/include/opencv4
LIBS += -L/home/vance/mine/2019/opencv-4.11.0/build/install/lib \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_videoio \
        -lopencv_imgcodecs \
        -lopencv_dnn \
        -lopencv_calib3d \
        -lopencv_features2d \
        -lopencv_flann

# ONNX Runtime-arm32
INCLUDEPATH += /home/vance/mine/tools/onnxruntime-linux-arm-1.16.3/include
LIBS += /usr/lib/libonnxruntime.so.1.16.3  # 完整路径

LIBS +=/usr/lib/libQt5WebEngineCore.so.5.12.9

TARGET = prs
TEMPLATE = app


CONFIG += c++17

SOURCES += \
    inference.cpp \
    main.cpp \
    widget.cpp

HEADERS += \
    inference.h \
    widget.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# RESOURCES += \
#     img.qrc

