#include "socksGUI.h"
#include "Para.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator oTranslator;
    bool IsLoad = oTranslator.load(":/qt_zh_CN"); // ע��˴��ַ����ԡ�:/����ͷ����ӵ��ַ����ǸղŸ��Ƶ�qm�ļ�������
    if (IsLoad) {
        a.installTranslator(&oTranslator); //����qt�Դ��ķ����ļ�
    }
    Para win;
    win.show();
    return a.exec();
}
