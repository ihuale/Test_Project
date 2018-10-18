#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Test_tf_speed.h"

class Test_tf_speed : public QMainWindow
{
	Q_OBJECT

public:
	Test_tf_speed(QWidget *parent = Q_NULLPTR);

private:
	Ui::Test_tf_speedClass ui;
};
