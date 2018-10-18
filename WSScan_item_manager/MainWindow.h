#pragma once
#include <QtWidgets/QMainWindow>
#include <QAction>

#include "ui_MainWindow.h"
#include "ImageView.h"
#include "head.h"


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);

public slots:
	//arg_time: ms
	void onShowMessage(std::string arg_mes,int arg_time);

	void scaleScene(double arg_rate);

	void zoomIn();
	void zoomOut();

private:
	Ui::MainWindow ui;

	ImageView *m_view;

	QAction *m_action_next;
	QAction *m_action_zoom_out;
	QAction *m_action_zoom_in;

	double m_scene_scale_current;
};
