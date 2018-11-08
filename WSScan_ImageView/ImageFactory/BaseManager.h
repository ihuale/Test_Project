#pragma once
#include <string>
#include <QObject>
//#include "ImageFactory/Parampack.h"

class QMouseEvent;
class QKeyEvent;

class BaseManager :public QObject
{
	Q_OBJECT
public:
	virtual std::string name() = 0;
	virtual void setMode(int arg) = 0;//what the specific code represents is determined by itself
	//virtual void setParampack(ParampackSPTR arg) = 0;
	virtual void cleanManager() = 0;

	virtual void mouseMoveEvent(QMouseEvent *event) = 0;
	virtual void mousePressEvent(QMouseEvent *event) = 0;
	virtual void mouseReleaseEvent(QMouseEvent *event) = 0;
	virtual void mouseDoubleClickEvent(QMouseEvent *event) = 0;
	virtual void keyPressEvent(QKeyEvent *event) = 0;
};