#include "socksGUI.h"
#include "Para.h"
#include <QDebug>
#include <modbus.h>
#include <bitset>
#include <QRegularExpression>
#include <QTimer>

int mytime;//通讯定时
modbus_t* ctx; //Modbus TCP / IP初始化
extern std::vector<double>Sockdataplus; //获取通讯数据

socksGUI::socksGUI(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    
    //按键禁用 互锁
    ui.pushButtonCloseCam->setDisabled(true);
    ui.pushButtonSoftTrig->setDisabled(true);
    ui.pushButtonStartCam->setDisabled(true);
    ui.pushButtonStopCam->setDisabled(true);
    ui.frame_2->setDisabled(true);
    ui.push_connectPLC->setDisabled(true);
    ui.push_ModbusTCP->setDisabled(true);
    ui.push_ModbusTCPStop->setDisabled(true);

    camera = new BaslerCamera();

    ui.comboBox_mode->addItem("Off");
    ui.comboBox_mode->addItem("On");

    ui.comboBox_trigsrc->addItem("Software");
    ui.comboBox_trigsrc->addItem("Line1");

    connect(camera, SIGNAL(showImageSignal(QImage)), this, SLOT(showImage(QImage)));

    //通讯定时
    QTimer* timer = new QTimer(this);
    connect(ui.push_ModbusTCP, &QPushButton::clicked, [=]() 
        {
        timer->start(mytime);
        connect(timer, &QTimer::timeout, this, [=]() 
            {
                ModbusTCP();
            });
        });
    //通讯停止
    connect(ui.push_ModbusTCPStop, &QPushButton::clicked, [=]()
        {
            timer->stop();
        });

}

socksGUI::~socksGUI()
{
    delete camera, camera = nullptr;
}

//返回参数设置界面
void socksGUI::back()
{
    QMessageBox::StandardButton result = QMessageBox::question(this, "提示", "确定返回参数设置界面? ", QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes)
    {
        Para* win = new Para;//创建参数设置界面
        win->show();//显示参数设置界面
        delete this;//把主界面删除
    }
    else {}
}
//通讯定时
void socksGUI::time_finish()
{
    bool ok;
    //获取lineedit数据
    QString time_s = ui.time_Edit->text();
    int time = time_s.toInt(&ok);
    if (time *1000 <= 0)
    {
        ui.time_Edit->setStyleSheet("QLineEdit { color: red; }");
    }
    else
    {
        ui.time_Edit->setStyleSheet("QLineEdit { color: black; }");
    }
    mytime = time * 1000;
}
//判断IP合法性
bool socksGUI::isIPValid(const QString& ip) {
    QRegularExpression ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QRegularExpressionMatch match = ipRegex.match(ip);
    return match.hasMatch();
}
//获取IP地址
QString socksGUI::IP_change()
{
    //获取lineedit数据
    QString IP = ui.IP->text();
    if (isIPValid(IP)) {
        //qDebug() << "IP地址有效";
        // IP地址合法
        ui.IP->setStyleSheet("QLineEdit { color: black; }");
    }
    else {
        /*qDebug() << "IP地址无效";*/
        // IP地址不合法
        ui.IP->setStyleSheet("QLineEdit { color: red; }");
    }
    return IP;
}
//获取端口号
int socksGUI::port_change()
{
    bool ok;
    //获取combobox数据
    QString port_s = ui.port->currentText();
    int port = port_s.toInt(&ok);
    return port;
}
//连接PLC
bool socksGUI::connectPLC()
{
    QString IP_Q = IP_change();//获取IP

    //QString转换为 const char*
    QByteArray byteArray = IP_Q.toUtf8();
    const char* IP_c = byteArray.constData();
    //qDebug() << "IP_c Value is:" << IP_c;

    int port = port_change();//获取端口号
    //qDebug() << "port Value is:" << port;

    if (IP_Q.isEmpty())
    {
        // 设置LineEdit变为红色
        ui.IP->setStyleSheet("QLineEdit{border:1px solid red }");
        QMessageBox::warning(this, "警告", "IP 不能为空!");
        return -1;
    }
    else if (mytime <= 0)
    {
        // 设置LineEdit变为红色
        ui.time_Edit->setStyleSheet("QLineEdit{border:1px solid red }");
        QMessageBox::warning(this, "警告", "通讯定时不能为空!");
        return -1;
    }
    else
    {
        // 创建一个Modbus TCP/IP连接
        ctx = modbus_new_tcp(IP_c, port);
        if (ctx == NULL)
        {
            //qDebug() << "Unable to create modbus_t object!" << modbus_strerror(errno);
            QMessageBox::warning(this, "警告", "Modbus TCP/IP 连接创建失败!");
            return -1;
        }

        // 连接到PLC
        if (modbus_connect(ctx) == -1)
        {
            //qDebug() << "Failed to connect to PLC!" << modbus_strerror(errno);
            QMessageBox::warning(this, "警告", "PLC 连接失败!");
            modbus_free(ctx);
            return -1;
        }
        QMessageBox* box = new QMessageBox(QMessageBox::Information, "提示", "PLC 连接成功!", QMessageBox::Ok);
        //1000ms后用户未作选择，则自动取消
        QTimer::singleShot(1000, box, SLOT(accept()));
        if (box->exec() == QMessageBox::Ok) {
            //用户选择了Ok后的操作
            delete box;
            box = NULL;
        }
        //new完后delete，防止内存泄漏
        delete box;
        box = NULL;
        ui.push_ModbusTCP->setEnabled(true);
        ui.push_connectPLC->setEnabled(false);
        ui.IP->setEnabled(false);
        ui.port->setEnabled(false);
        ui.time_Edit->setEnabled(false);
        return 1;
    }
}
//开始通讯
bool socksGUI::ModbusTCP()
{
    ui.push_ModbusTCP->setEnabled(false);
    ui.push_ModbusTCPStop->setEnabled(true);
    ui.pushButtonStopCam->setEnabled(false);
  
    //设置PCL地址
    modbus_set_slave(ctx, 1);
    uint16_t data[8];
    int g = 0;
    for (int i = 0; i < Sockdataplus.size(); i++)
    {
        data[g] = Sockdataplus[i];
        g++;
        /*std::cout << Sockdata[i] << ' ';*/
    }
    data[6] = Sockdataplus.size();
    data[7] = 1;
    std::cout << "Sock数据：";
    for (int i = 0; i < 8; i++)
    {
        std::cout << data[i] << ",";
    }

    // 将数据发送到PLC
    int num_regs = 8;
    int write_addr = 0;  // 这里的地址可以根据PLC的需要进行修改
    if (modbus_write_registers(ctx, write_addr, num_regs, data) == -1) {
        //qDebug() << "Unable to write data to PLC!" << modbus_strerror(errno);
        QMessageBox::warning(this, "警告", "数据写入 PLC 失败!");
        modbus_free(ctx);
        return -1;
    }
    qDebug() << "Data write success!";

    //显示通讯内容
    ui.y1_Edit->setText(QString::number(Sockdataplus[0]));
    ui.x_Edit->setText(QString::number(Sockdataplus[1]));
    ui.y2_Edit->setText(QString::number(Sockdataplus[2]));
    ui.dy_Edit->setText(QString::number(Sockdataplus[3]));
    ui.a_Edit->setText(QString::number(Sockdataplus[4]));
    ui.sym_Edit->setText(QString::number(Sockdataplus[5]));
    //QMessageBox::information(this, "Hint", "Data write success!");
    return 1;
}
//停止通讯
bool socksGUI::ModbusTCPStop()
{
    // 断开与PLC的连接
    modbus_close(ctx);
    modbus_free(ctx);
    ui.push_connectPLC->setEnabled(true);
    ui.push_ModbusTCPStop->setEnabled(false);
    ui.pushButtonStopCam->setEnabled(true);
    ui.IP->setEnabled(true);
    ui.port->setEnabled(true);
    ui.time_Edit->setEnabled(true);
    return 1;
}
//打开相机
void socksGUI::on_pushButtonOpenCam_clicked()
{
    if (camera->OpenCamera()) {

        ui.pushButtonCloseCam->setEnabled(true);
        ui.pushButtonSoftTrig->setEnabled(true);
        ui.pushButtonStartCam->setEnabled(true);
        ui.pushButtonStopCam->setEnabled(false);
        ui.frame_2->setEnabled(true);

        std::string tmp;
        INT64 exp;
        camera->GetIntPara("ExposureTimeRaw", exp);
        ui.spinBoxExp->setValue(exp);
        ui.horizontalSliderExp->setValue(exp);

        camera->GetStringPara("TriggerMode", tmp);
        qDebug() << tmp.c_str();
        int index = ui.comboBox_mode->findText(tmp.c_str());
        ui.comboBox_mode->setCurrentIndex(index);

        camera->GetStringPara("TriggerSource", tmp);
        qDebug() << tmp.c_str();
        index = ui.comboBox_trigsrc->findText(tmp.c_str());
        ui.comboBox_trigsrc->setCurrentIndex(index);
    }
}
//开始拍照
void socksGUI::on_pushButtonStartCam_clicked()
{
    camera->start();
    ui.pushButtonStartCam->setEnabled(false);
    ui.pushButtonStopCam->setEnabled(true);
    ui.pushButtonCloseCam->setEnabled(false);
    ui.push_back->setEnabled(false);
    ui.push_connectPLC->setEnabled(true);
}
//停止拍照
void socksGUI::on_pushButtonStopCam_clicked()
{
    camera->StopCamera();
    // 等待一段时间
    Sleep(1000);
    ui.pushButtonStartCam->setEnabled(true);
    ui.pushButtonStopCam->setEnabled(false);
    ui.pushButtonCloseCam->setEnabled(true);
    ui.push_back->setEnabled(true);
    ui.push_connectPLC->setEnabled(false);
}
//关闭相机
void socksGUI::on_pushButtonCloseCam_clicked()
{
    camera->CloseCamera();

    ui.pushButtonCloseCam->setDisabled(true);
    ui.pushButtonSoftTrig->setDisabled(true);
    ui.pushButtonStartCam->setDisabled(true);
    ui.pushButtonStopCam->setDisabled(true);
    ui.frame_2->setDisabled(true);
}
//实时显示
void socksGUI::showImage(const QImage& img)
{
    ui.label_video->setPixmap(QPixmap::fromImage(img).scaled(ui.label_video->width(),
        ui.label_video->height(),
        Qt::KeepAspectRatioByExpanding));
}
//修改曝光 滑动
void socksGUI::on_horizontalSliderExp_valueChanged(const int& value)
{
    camera->SetIntPara("ExposureTimeRaw", value);
    ui.spinBoxExp->setValue(value);
}
//修改曝光 微调
void socksGUI::on_spinBoxExp_editingFinished()
{
    int tmp = ui.spinBoxExp->value();
    camera->SetIntPara("ExposureTimeRaw", tmp);
    ui.horizontalSliderExp->setValue(tmp);
}
//软件触发
void socksGUI::on_pushButtonSoftTrig_clicked()
{
    camera->SetCmd("TriggerSoftware");
}
//触发模式
void socksGUI::on_comboBox_mode_activated(const QString& arg1)
{
    camera->SetStringPara("TriggerMode", arg1.toStdString().c_str());
}
//触发源
void socksGUI::on_comboBox_trigsrc_activated(const QString& arg1)
{
    camera->SetStringPara("TriggerSource", arg1.toStdString().c_str());
}
//退出
void socksGUI::myclose()
{
    QMessageBox::StandardButton result = QMessageBox::question(this, "提示", "确定退出?", QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes)
    {
        this->close();
    }
    else {}
}