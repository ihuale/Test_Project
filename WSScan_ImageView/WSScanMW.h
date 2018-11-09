#pragma once

#include <QAction>
#include <QFileInfoList>
#include <QtWidgets/QMainWindow>
#include "ui_WSScanMW.h"
#include "UI/Viewer.h"
#include "ImageFactory/OpenSlideImage.h"

class WSScanMW : public QMainWindow
{
	Q_OBJECT

public:
	WSScanMW(QWidget *parent = Q_NULLPTR);
	~WSScanMW();

	bool loadDirFiles(std::string arg_path,std::string arg_filter);

public slots:
	void onActionChangeImage();

private:
	Ui::WSScanMW ui;

	Viewer* m_viewer;
	OpenSlideImageSP m_img;

	QAction* actionChangeImage;

	QFileInfoList list_file;
	int index_currretn;
};
