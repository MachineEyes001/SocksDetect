#include "Para.h"
#include "socksGUI.h"
#include "Blacksock_pop.h"

// 定义用户参数
int dx;
int ribtop_width;
int ribtop_height;
double con;
int type;
int num;
extern int h;
int black_h;

Para::Para(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
    setFixedSize(502, 340);
}

Para::~Para()
{}

//判断输入是否合法
int Para::dx_finish()
{
    bool ok;
    //获取lineedit数据
    QString dx_s = ui.dx->text();
    int dx = dx_s.toInt(&ok);
    if (dx <= 0)
    {
        ui.dx->setStyleSheet("QLineEdit { color: red; }");
    }
    else
    {
        ui.dx->setStyleSheet("QLineEdit { color: black; }");
    }
    return dx;
}
int Para::ribtop_width_finish()
{
    bool ok;
    //获取lineedit数据
    QString ribtop_width_s = ui.ribtop_width->text();
    int ribtop_width = ribtop_width_s.toInt(&ok);
    if (ribtop_width <= 0)
    {
        ui.ribtop_width->setStyleSheet("QLineEdit { color: red; }");
    }
    else
    {
        ui.ribtop_width->setStyleSheet("QLineEdit { color: black; }");
    }
    return ribtop_width;
}
int Para::ribtop_height_finish()
{
    bool ok;
    //获取lineedit数据
    QString ribtop_height_s = ui.ribtop_height->text();
    int ribtop_height = ribtop_height_s.toInt(&ok);
    if (ribtop_height <= 0)
    {
        ui.ribtop_height->setStyleSheet("QLineEdit { color: red; }");
    }
    else
    {
        ui.ribtop_height->setStyleSheet("QLineEdit { color: black; }");
    }
    return ribtop_height;
}
double Para::con_finish()
{
    bool ok;
    //获取lineedit数据
    QString con_s = ui.con->text();
    double con = con_s.toDouble(&ok);
    if (con <= 0)
    {
        ui.con->setStyleSheet("QLineEdit { color: red; }");
    }
    else
    {
        ui.con->setStyleSheet("QLineEdit { color: black; }");
    }
    return con;
}
void Para::type_change()
{
    //获取combobox索引
    int type_i = ui.type_c->currentIndex();//获得索引
    ui.num_c->setEnabled(true);
    if (type_i == 1)
    {
        ui.num_c->setEnabled(false);
        this->setEnabled(false);//主界面失能
        //创建设置界面并且把this传入设置界面用于后面返回
        Blacksock_pop* win = new Blacksock_pop(this);
        win->show();
    }   
    type = type_i;
}
void Para::num_change()
{
    bool ok;
    //获取combobox数据
    QString num_s = ui.num_c->currentText();
    int num_i = num_s.toInt(&ok);
    num = num_i;
}
//确认进入到主界面
void Para::commit()
{
    con = con_finish();
    dx = dx_finish() * con;
    ribtop_width = ribtop_width_finish() * con;
    ribtop_height = ribtop_height_finish() * con;
    black_h = h * con;
    if (dx > 0 && ribtop_width > 0 && ribtop_height > 0 && con > 0)
    {
        //创建主界面
        socksGUI* win = new socksGUI;
        win->show();
        this->close();//这里不能用delete，因为this是main函数中创建的栈空间系统自动释放
    }
    else
    {
        QMessageBox::warning(this, "警告", "请正确输入初始化参数!");
    }
}
//取消
void Para::myclose()
{
    QMessageBox::StandardButton result = QMessageBox::question(this, "提示", "确定退出?", QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes)
    {
        this->close();
    }
    else{}
}

