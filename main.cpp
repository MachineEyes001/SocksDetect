#include "socksGUI.h"
#include "Para.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator oTranslator;
    bool IsLoad = oTranslator.load(":/qt_zh_CN"); // 注意此处字符串以“:/”开头，后接的字符串是刚才复制的qm文件的名字
    if (IsLoad) {
        a.installTranslator(&oTranslator); //加载qt自带的翻译文件
    }
    Para win;
    win.show();
    return a.exec();
}
