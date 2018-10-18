#pragma once
#include <string>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QReadWriteLock>
#include <QFileInfoList>
#include <QTimer>

#include "head.h"

class BboxItem;
class BboxManager;

class ImageView :
	public QGraphicsView
{
	Q_OBJECT
public:
	explicit ImageView(QWidget *parent);
	virtual ~ImageView();

	//set dir of image for load
	bool setImagePath(std::string arg_path);

	bool loadAllImageName();
	bool displayImage(ImageID arg_id);

public slots:
	//action to display next image
	void onDisplayNextImage();

signals:
	void showStatus(std::string arg_mes, int arg_time);

private:
	std::string m_path_image;

	//for test
	QReadWriteLock m_lock_rw;
	QGraphicsScene *m_scene;

	//dir
	QFileInfoList m_list_image_info;

	//for display
	QGraphicsPixmapItem *m_item;

	//for test
	int m_count = 1;
	QTimer *m_timer;

	//for test
	BboxItem *m_item_bbox;
	BboxManager *m_ite_manager;
};

