#include "ImageView.h"
#include <iostream>
#include <algorithm>
#include <QDir>
#include <QString>
#include <QFileInfoList>
#include <QPixmap>
#include <QPointF>
#include <QGraphicsPixmapItem>

#include "Item/BboxItem.h"//for test
#include "Item/BboxManager.h"

ImageView::ImageView(QWidget *parent):
	QGraphicsView(parent),
	m_path_image(path_image_read)
{
	//m_path_image is like: /ScanResults
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setResizeAnchor(QGraphicsView::ViewportAnchor::AnchorViewCenter);
	setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
	//setContentsMargins(0, 0, 0, 0);
	setAutoFillBackground(true);
	setViewportUpdateMode(ViewportUpdateMode::FullViewportUpdate);
	setInteractive(true);
	setBackgroundBrush(QBrush(QColor(252, 252, 252)));
	setContextMenuPolicy(Qt::CustomContextMenu);

	resize(1200, 800);

	m_scene = new QGraphicsScene(this);
	setScene(m_scene);//memory leak!!!
	//scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
	scene()->setBackgroundBrush(QBrush(QColor(252, 252, 252)));

	m_item = new QGraphicsPixmapItem;
	m_scene->addItem(m_item);
	m_scene->update();

	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, &ImageView::onDisplayNextImage);
	//m_timer->start(150);

	/*BboxInfo tem_info = { QPointF(100,100),100,100,2,0.65,3 };
	BboxItem *item_bbox = new BboxItem(tem_info, "#39FF33", this);
	m_scene->addItem(item_bbox);
	item_bbox->setPos(100, 100);
	this->update();*/

	m_ite_manager = new BboxManager(this);
	//m_ite_manager->onVisionChanged("./Image/", 3);
}


ImageView::~ImageView()
{
	m_scene->removeItem(m_item);
	delete m_item;
	delete m_ite_manager;
}

bool ImageView::setImagePath(std::string arg_path)
{
	if (arg_path.empty()) {
		std::cout << "[ImageView] setImagePath: path is empty: " << arg_path << std::endl;
		return false;
	}
	QDir tem_dir(arg_path.c_str());
	if (!tem_dir.exists()) {
		std::cout << "[ImageView] setImagePath: path is not exits: " << arg_path << std::endl;
		return false;
	}
	m_lock_rw.lockForWrite();
	m_path_image = arg_path;
	m_lock_rw.unlock();

	std::cout << "[ImageView] setImagePath: set iamge path: " << arg_path << std::endl;

	return true;
}

bool ImageView::loadAllImageName()
{

	QDir tem_dir((m_path_image + "Images").c_str());
	if (!tem_dir.exists()) {
		std::cout << "[ImageView] loadAllImageName: path is not exits: " 
			<< m_path_image + "Images" << std::endl;
		return false;
	}
	QStringList filters;
	filters << QString("*.jpg");
	tem_dir.setFilter(QDir::Files | QDir::NoSymLinks);
	tem_dir.setNameFilters(filters);

	//get image info
	if (!m_list_image_info.empty()) {
		m_list_image_info.clear();
	}

	m_list_image_info =
		tem_dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	if (m_list_image_info.empty()) {
		std::cout << "[ImageView] loadAllImageName: no file in path:" 
			<< m_path_image + "Images" << std::endl;
		return false;
	}

	std::sort(m_list_image_info.rbegin(), m_list_image_info.rend(), 
		[&](QFileInfo &arg_1, QFileInfo &arg_2) {
		return arg_1.fileName() > arg_2.fileName(); });

	//just only display the first exits image
	displayImage(1);

	return true;
}

bool ImageView::displayImage(ImageID arg_id)
{
	if (m_list_image_info.size() < 1) {
		return false;
	}
	QString tem_filename = QString::number(arg_id) + ".jpg";
	bool tem_flag = false;
	for (auto &tem_iter : m_list_image_info) {
		//TODO
		//here should be find,not compare!!
		if (tem_iter.fileName() != tem_filename) {
			//not this one
			continue;
		}
		QFileInfo info(tem_iter.absoluteFilePath());
		if (!info.exists()) {
			std::cout << "[ImageView] displayImage: no file:"
				<< tem_iter.fileName().toStdString() << std::endl;
			//found but not exists
			break;
		}

		//load bbox
		m_ite_manager->onVisionChanged(m_path_image, arg_id);

		//found and exists,then display it
		tem_flag = true;
		QString fileName = tem_iter.absoluteFilePath();
		QPixmap tem_pixmap;
		if (!tem_pixmap.load(fileName)) {
			std::cout<<"[ImageView] load file to pixmap failed: " 
				<< tem_iter.fileName().toStdString() << std::endl;
			return false;
		}
		//QGraphicsPixmapItem tem_item(tem_pixmap);
		m_scene->removeItem(m_item);
		m_item = new QGraphicsPixmapItem(tem_pixmap);
		m_scene->addItem(m_item);
		m_item->setZValue(0.1);
		m_scene->update();
		//just only display the first exits image

		std::cout << "[ImageView] display image: "
			<< tem_iter.absoluteFilePath().toStdString() << std::endl;
		emit showStatus(tem_iter.fileName().toStdString(), 1000);
		break;
	}
	return tem_flag;
}


void ImageView::onDisplayNextImage()
{
	if (m_list_image_info.size() < 1) {
		std::cout << "[ImageView] onDisplayNextImage: no image" << std::endl;
		return;
	}
	if (m_count > 2496) {
		m_count = 0;
	}
	else {
		++m_count;
	}
	
	displayImage(m_count);
}