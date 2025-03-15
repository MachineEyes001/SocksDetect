#pragma once
#include <QMainWindow>
#include "ui_Blacksock_pop.h"
#include<QMessageBox>

class Blacksock_pop : public QMainWindow
{
	Q_OBJECT

public:
	Blacksock_pop(QWidget *parent = nullptr);
	~Blacksock_pop();

private:
	Ui::Blacksock_popClass ui;

private slots:
	int Blacksock_finish();
	void commit();
	void mycancel();
};
