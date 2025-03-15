#include "Blacksock_pop.h"

int h;

Blacksock_pop::Blacksock_pop(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setFixedSize(272, 140);
}

Blacksock_pop::~Blacksock_pop()
{}

int Blacksock_pop::Blacksock_finish()
{
    bool ok;
    //获取lineedit数据
    QString h_s = ui.h->text();
    int h = h_s.toInt(&ok);
    if (h <= 0)
    {
        ui.h->setStyleSheet("QLineEdit { color: red; }");
    }
    else
    {
        ui.h->setStyleSheet("QLineEdit { color: black; }");
    }
    return h;
}

void Blacksock_pop::commit()
{
    h = Blacksock_finish();
    if (h > 0)
    {
        this->parentWidget()->setEnabled(true); //主界面使能
        delete this;//释放设置界面 也可以用this->deleteLater()但是这个函数不会立即释放空间
    }
    else
    {
        QMessageBox::warning(this, "警告", "请正确输入高度参数!");
    }
}

void Blacksock_pop::mycancel()
{
    this->parentWidget()->setEnabled(true); //主界面使能
    delete this;//释放设置界面 也可以用this->deleteLater()但是这个函数不会立即释放空间
}