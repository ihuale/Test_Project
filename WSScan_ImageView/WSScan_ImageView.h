#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_WSScan_ImageView.h"
#include "UI/Viewer.h"
#include "ImageFactory/OpenSlideImage.h"

class WSScan_ImageView : public QMainWindow
{
	Q_OBJECT

public:
	WSScan_ImageView(QWidget *parent = Q_NULLPTR);

private:
	Ui::WSScan_ImageViewClass ui;

	Viewer* m_viewer;
	OpenSlideImageSP m_img;
};
