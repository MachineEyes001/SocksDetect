#pragma once
#include <QMainWindow>
#include "ui_Para.h"
#include<QMessageBox>
#include <QProcess>

class Para : public QMainWindow
{
	Q_OBJECT

public:
	Para(QWidget *parent = nullptr);
	~Para();

private:
	Ui::ParaClass ui;

private slots:
	int dx_finish();
	int ribtop_width_finish();
	int ribtop_height_finish();
	double con_finish();
	void type_change();
	void num_change();
	void commit();//确认按钮
	void myclose();//取消按钮
};
