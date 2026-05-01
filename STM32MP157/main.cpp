#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    // 设置Qt WebEngine的沙箱参数
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox");

    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}


/*
前三个文件是stm32f103c8t6的串口文件，功能是把温湿度、光敏、热值、热敏通过串口以文本数据包形式发送，
后面四个文件是stm32mp157的qt程序，qt程序经过交叉编译生成可执行文件拷贝到stm32mp157运行，修改后面四个文件，
实现在tab2页面的label_3显示串口传来的数据包
*/
