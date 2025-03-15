/*********************************************
*   说明：Basler相机主界面
**********************************************/
#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QtWidgets/QMainWindow>
#include "BaslerCamera.h"
#include "ui_socksGUI.h"
#include <vector>
#include <QTimer>
#include <QTranslator.h>
#include<QMessageBox>

class socksGUI : public QMainWindow
{
    Q_OBJECT

public:
    socksGUI(QWidget *parent = nullptr);
    ~socksGUI();

private:
    Ui::socksGUIClass ui;
    BaslerCamera* camera;

private slots:
    void on_pushButtonOpenCam_clicked();
    void on_pushButtonStartCam_clicked();
    void on_pushButtonStopCam_clicked();
    void on_pushButtonCloseCam_clicked();
    void showImage(const QImage& img);
    void on_horizontalSliderExp_valueChanged(const int& value);
    void on_pushButtonSoftTrig_clicked();
    void on_comboBox_mode_activated(const QString& arg1);
    void on_comboBox_trigsrc_activated(const QString& arg1);
    void on_spinBoxExp_editingFinished();
    void back();
    void myclose();
    bool connectPLC();
    bool ModbusTCP();
    bool ModbusTCPStop();
    bool isIPValid(const QString& ip);
    QString IP_change();
    int port_change();
    void time_finish();

};