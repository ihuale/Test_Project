#include <iostream>
#include <iterator>
#include <vector>
#include <list>
#include <memory>
#include <cmath>
#include <windows.h>
#include <QPixmap>
#include <QMenu>
#include <QObject>
#include <QAction>
#include <QTimeLine>
#include <QStatusBar>
#include <QScrollBar>
#include <QMainWindow>
#include <QResizeEvent>
#include <QTime>
#include <QFileinfo>
#include <QFileDialog>
#include <QDebug>

#include "Viewer.h"
#include "cache/TileManager.h"
#include "cache/RenderThread.h"
#include "cache/RenderWorker.h"
#include "cache/TileGraphicsItemCache.h"
#include "ImageFactory/BaseManager.h"
#include "ImageFactory/TileGraphicsItem.h"

#define GETVISIBLEPOS(res, pos1, pos2, mm1, mm2) QPointF res = QPointF((std::mm1)(globalImgRect_.pos1(), FOV_.pos1()), (std::mm2)(globalImgRect_.pos2(), FOV_.pos2()));

Viewer::Viewer(QWidget *parent) :
	QGraphicsView(parent),
	lastLevel(0),
	_numScheduledScalings(0),
	oldLevel(0),
	_sceneScale(1),
	currentLevelScale(1),
	_prevPan(0, 0),
	isPanning(false)
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setResizeAnchor(QGraphicsView::ViewportAnchor::AnchorViewCenter);
	setDragMode(QGraphicsView::DragMode::NoDrag);
	setContentsMargins(0, 0, 0, 0);
	setAutoFillBackground(true);
	setViewportUpdateMode(ViewportUpdateMode::FullViewportUpdate);
	setInteractive(true);
	this->setScene(new QGraphicsScene);
	scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
	this->setBackgroundBrush(QBrush(QColor(252, 252, 252)));
	this->scene()->setBackgroundBrush(QBrush(QColor(252, 252, 252)));
	this->setContextMenuPolicy(Qt::CustomContextMenu);

	temItem = 0;
	m_tileManager = 0;

	zoomTimer = new QTimer(this);//not used!!!
	connect(zoomTimer, SIGNAL(timeout()), this, SLOT(StopZoomTimer()));
	connect(this, SIGNAL(viewOfFieldChanged()), this, SLOT(onViewOfFieldChanged()));

	this->resize(1200, 800);
	this->setEnabled(false);
}

Viewer::~Viewer()
{
	qDebug() << "Viewer: ~Viewer() 0";
	close();
	qDebug() << "Viewer: ~Viewer() 1";
}

void Viewer::initialize(OpenSlideImageSP img)
{
	/*close();*/
	setEnabled(true);
	m_img = img;
	if (m_tileManager) {
		m_tileManager->clear();
	}
	m_tileManager = new TileManager(m_img, this);
	m_tileManager->m_tileSize = 512.;
	double tileSize = m_tileManager->m_tileSize;
	lastLevel = img->m_levels - 1;
	for (int i = lastLevel; i >= 0; --i) {
		std::vector<unsigned long long> lastLevelDimensions = img->m_dims[i];
		if (lastLevelDimensions[0] > tileSize && lastLevelDimensions[1] > tileSize) {
			lastLevel = i;
			img->m_currentLevel = i;
			img->m_lastRenderLevel = i;
			break;
		}
	}

	m_tileManager = new TileManager(m_img, this);//tileSize, lastLevel,
	setMouseTracking(true);
	
	oldLevel = img->m_currentLevel;
	_sceneScale = 1. / m_img->getLevelDownsample(lastLevel);
	QImage temImg;
	m_img->readDataFromImage(0, 0, img->m_dims[lastLevel][0], img->m_dims[lastLevel][1], lastLevel, temImg);
	QPixmap renderPixmap = QPixmap::fromImage(temImg);
	//renderPixmap->convertFromImage(temImg);
	qDebug() << "[Viewer] read pixmap:" << renderPixmap;
	temItem = new TileGraphicsItem(&renderPixmap,
		img->m_dims[lastLevel][0],
		img->m_dims[lastLevel][1],
		0, 0, 
		lastLevel, lastLevel,
		img->getLevelScaleRate());

	float longest = img->m_dims[lastLevel][0] > img->m_dims[lastLevel][1] ? img->m_dims[lastLevel][0] : img->m_dims[lastLevel][1];
	_sceneScale = 1. / m_img->getLevelDownsample(lastLevel);
	QRectF n((img->m_dims[lastLevel][0] / 2) - 1.5*longest, (img->m_dims[lastLevel][1] / 2) - 1.5*longest, 3 * longest, 3 * longest);
	this->setSceneRect(n);
	this->fitInView(QRectF(0, 0, img->m_dims[lastLevel][0], img->m_dims[lastLevel][1]), Qt::AspectRatioMode::KeepAspectRatio);
	//this->scene()->addItem(temItem);
	//this->centerOn(temItem);
	this->show();
	globalImgRect_ = temItem->boundingRect();

	//delete temItem; 
	//temItem = 0;
	onViewOfFieldChanged();
}

void Viewer::resizeEvent(QResizeEvent *event)
{
	//do something
	if (m_img) {
		getBoundingPoint();
		calcVisibleLevel();
		emit viewOfFieldChanged();
	}

	QGraphicsView::resizeEvent(event);
}

void Viewer::wheelEvent(QWheelEvent *event) {
	int numDegrees = event->delta() / 8;
	_zoomToScenePos = this->mapToScene(event->pos());
	_zoomToViewPos = event->pos();
	numSteps = numDegrees / 15;
	zoom(numSteps);
}

void Viewer::keyPressEvent(QKeyEvent *event)
{
	if (!this->isEnabled()) return;

	if (m_img) {
		if (event->key() == Qt::Key_Left 
			|| event->key() == Qt::Key_Right 
			|| event->key() == Qt::Key_Up 
			|| event->key() == Qt::Key_Down ) {
			QGraphicsView::keyPressEvent(event);
			emit viewOfFieldChanged();
		}
	}
	if (_activemanager) {
		_activemanager->keyPressEvent(event);
		event->accept();
	}
	//QGraphicsView::keyPressEvent(event);
	event->accept();
}

void Viewer::mouseMoveEvent(QMouseEvent *event)
{
	QPointF imgLoc = this->mapToScene(event->pos());
	QString temStr=("Current position: (") + QString::number(imgLoc.x()) + QString(", ") + QString::number(imgLoc.y()) + QString(")");
	emit parentShowCoordinate(temStr.toStdString(), 1000);

	if (isPanning) {
		pan(event->pos());
		mouseMoveTriggerFlag_ = (mouseMoveTriggerFlag_ >> 7) | (mouseMoveTriggerFlag_ << 1);
		if (mouseMoveTriggerFlag_ & 0x44){
			emit viewOfFieldChanged();
		}
		event->accept();
		return;
	}
	if (_activemanager) {
		_activemanager->mouseMoveEvent(event);
	}
	event->ignore();
}

void Viewer::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MouseButton::MidButton) 
	{
		togglePan(true, event->pos());
		mouseMoveTriggerFlag_ = 1;
		event->accept();
	}
	if (event->button() == Qt::LeftButton)
	{
		if (_activemanager) {
			_activemanager->mousePressEvent(event);
			event->accept();
		}
		else {
			QGraphicsView::mousePressEvent(event);
			event->accept();
		}
	}
	else {
		QGraphicsView::mousePressEvent(event);
		event->accept();
	}
}

void Viewer::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MouseButton::MidButton)
	{
		togglePan(false);
		event->accept();
		mouseMoveTriggerFlag_ = 1;
		emit viewOfFieldChanged();
	}
	if (_activemanager) {
		_activemanager->mouseReleaseEvent(event);
	}
	QGraphicsView::mouseReleaseEvent(event);
	event->ignore();
}

void Viewer::mouseDoubleClickEvent(QMouseEvent * event)
{
	if (_activemanager) {
		_activemanager->mouseDoubleClickEvent(event);
	}
	event->ignore();
}

void Viewer::close()
{
	qDebug() << "Viewer: close() 0";
	if (m_tileManager) {
		m_tileManager->clear();
		//delete m_tileManager;//delete??
		//m_tileManager = NULL;
	}
	if (_activemanager) {
		_activemanager->cleanManager();
	}
	//delete _img;
	//_img = NULL;
	m_img.reset();
	scene()->clear();
	setEnabled(false);
	qDebug() << "Viewer: close() 1";
}

void Viewer::zoom(float numSteps) {
	if (!m_img) {
		return;
	}
	_numScheduledScalings += numSteps;
	if (_numScheduledScalings * numSteps < 0)  {
		_numScheduledScalings = numSteps;
	}
	
	QTimeLine *anim = new QTimeLine(300, this);
	anim->setUpdateInterval(5);
	connect(anim, SIGNAL(valueChanged(qreal)), this, SLOT(scalingTime(qreal)));
	connect(anim, SIGNAL(finished()), this, SLOT(zoomFinished()));
	anim->start();
}

void Viewer::togglePan(bool pan, const QPoint& startPos) {
	if (pan) {
		if (isPanning) {
			return;
		}
		isPanning = true;
		_prevPan = startPos;
		setCursor(Qt::ClosedHandCursor);
	}
	else {
		if (!isPanning) {
			return;
		}
		isPanning = false;
		_prevPan = QPoint(0, 0);
		setCursor(Qt::ArrowCursor);
	}
}

void Viewer::pan(QPoint panTo)
{
	QScrollBar *hBar = horizontalScrollBar();
	QScrollBar *vBar = verticalScrollBar();
	QPoint delta = panTo - _prevPan;
	_prevPan = panTo;
	hBar->setValue(hBar->value() + (isRightToLeft() ? delta.x() : -delta.x()));
	vBar->setValue(vBar->value() - delta.y());
	
}

void Viewer::scalingTime(qreal x)
{
	qreal factor = 1.0 + qreal(_numScheduledScalings) * x / 300.;
	float maxDownsample = std::pow(2.0, m_img->m_levels - m_img->m_currentLevel - 1);
	FOV_ = this->mapToScene(this->rect()).boundingRect();
	QRectF FOVImage = QRectF(FOV_.left() / this->_sceneScale, FOV_.top() / this->_sceneScale, FOV_.width() / this->_sceneScale, FOV_.height() / this->_sceneScale);
	float scaleX = static_cast<float>(m_img->m_dims[0][0]) / FOVImage.width();
	float scaleY = static_cast<float>(m_img->m_dims[0][1]) / FOVImage.height();
	float minScale = scaleX > scaleY ? scaleY : scaleX;
	float maxScale = scaleX > scaleY ? scaleX : scaleY;
	if ((factor < 1.0 && maxScale < 0.5) || (factor > 1.0 && minScale > maxDownsample)) {
		return;
	}
	scale(factor, factor);
	centerOn(_zoomToScenePos);
	QPointF delta_viewport_pos = _zoomToViewPos - QPointF(width() / 2.0, height() / 2.0);
	QPointF viewport_center = mapFromScene(_zoomToScenePos) - delta_viewport_pos;
	centerOn(mapToScene(viewport_center.toPoint()));
	emit viewOfFieldChanged();

}

void Viewer::zoomFinished()
{
	if (_numScheduledScalings > 0)
		_numScheduledScalings--;
	else
		_numScheduledScalings++;
	//zoomTimer->start(zoomTime);
}

void Viewer::StopZoomTimer()
{
	emit viewOfFieldChanged();
}

void Viewer::onViewOfFieldChanged()
{
	if (m_img) {
		getBoundingPoint();
		calcVisibleLevel();
		//splitImage();

		m_tileManager->loadTilesForFieldOfView(visibleRect_, m_img->m_currentLevel);
	}
}

void Viewer::readNewFOI()
{
	//this is function just for debug
	//_sceneScale = 1. / _img->getLevelDownsample(lastLevel);

	//QImage temImg;
	//_img->readDataFromImage(visibleRect_.x() / _sceneScale,//frame0
	//	visibleRect_.y() / _sceneScale,
	//	visibleRect_.width()*std::pow(2, lastLevel - currentLevel),
	//	visibleRect_.height()*std::pow(2, lastLevel - currentLevel),
	//	currentLevel,
	//	temImg);

	//QPixmap *renderPixmap = new QPixmap; renderPixmap->convertFromImage(temImg);
	//scene()->removeItem(temItem); 
	///*delete temItem;*/ //!!!!!!!!!
	//update();
	//temItem = new TileGraphicsItem(renderPixmap,
	//visibleRect_.width(),
	//visibleRect_.height(),
	//visibleRect_.width() * visibleRect_.height() * 4,
	//currentLevel, lastLevel, _parampack->getLevelScaleRate());

	//this->scene()->addItem(temItem);
	//temItem->setPos(visibleRect_.x(), visibleRect_.y());
	//if (currentLevel != 0) {
	//temItem->setZValue(1 / currentLevel);
	//}
	//else temItem->setZValue(0);

}

void Viewer::getBoundingPoint()
{
	FOV_ = this->mapToScene(this->rect()).boundingRect();
	if (FOV_.right() < 0 || FOV_.bottom() < 0) {
		visibleRect_.setCoords(0, 0, 0, 0);
		return;
	}
	GETVISIBLEPOS(_topLeft, left, top, max, max);
	GETVISIBLEPOS(_bottomRight, right, bottom, min, min);

	visibleRect_.setTopLeft(_topLeft);
	visibleRect_.setBottomRight(_bottomRight);
}

void Viewer::calcVisibleLevel()
{
	if (visibleRect_.right() < 0 || visibleRect_.bottom() < 0) {
		return;
	}
	if (ratio.size() != (lastLevel + 1))
		ratio.resize(lastLevel + 1);
	for (int i = 0; i < (lastLevel + 1); ++i) {
		auto tmp = visibleRect_.height() / globalImgRect_.height() * m_img->m_dims[0][1] / std::pow(2.0, i) / this->rect().height();
		if (tmp < 1.0) tmp = 1. / tmp;
		ratio[i] = tmp;
	}

	auto iter = std::min_element(ratio.begin(), ratio.end());
	oldLevel = m_img->m_currentLevel;
	m_img->m_currentLevel = std::distance(ratio.begin(), iter);
}

void Viewer::splitImage()
{
	//_sceneScale = 1. / _img->getLevelDownsample(lastLevel);
	//double den = _img->getLevelDownsample(currentLevel);
	//double upden = std::pow(2, lastLevel - currentLevel);

	////visibleRect is current view rect
	//QRectF visibleCurRect = QRectF(visibleRect_.x() * upden, 
	//	visibleRect_.y() * upden,
	//	visibleRect_.width() * upden,
	//	visibleRect_.height() * upden); // cur level

	//QPointF temTopLeft = visibleCurRect.topLeft();//cur level
	//double tileLevel0 = tileSize * den; // level 0 tile size
	//int viewTopLeftX = std::floor(temTopLeft.x() / tileSize);
	//int viewTopLeftY = std::floor(temTopLeft.y() / tileSize);
	//QPointF viewFirstTile = QPointF((viewTopLeftX ) * tileSize, (viewTopLeftY ) * tileSize);//cur level

	//int numOfViewX = std::ceil((visibleCurRect.x() + visibleCurRect.width()) / tileSize) - std::floor(visibleCurRect.x() / tileSize) + 1;
	//int numOfViewY = std::ceil((visibleCurRect.y() + visibleCurRect.height()) / tileSize) - std::floor(visibleCurRect.y() / tileSize) + 1;

	//std::vector<QRectF> tileRectList;
	//std::vector<std::string> keyList;
	//QRectF temRect;
	//
	//for (int index0 = 0; index0 < numOfViewX; ++index0) {
	//	for (int index1 = 0; index1 < numOfViewY; ++index1) {

	//		temRect = QRectF(viewFirstTile.x() * den + index0*tileLevel0, //global
	//			viewFirstTile.y() * den + index1*tileLevel0, 
	//			tileSize,
	//			tileSize);

	//		//0 level key
	//		//keyList.push_back((QString::number(viewFirstTile.x() * den + index0*tileLevel0) + "_"
	//		//	+ QString::number(viewFirstTile.y() * den + index1*tileLevel0) + "_"
	//		//	+ QString::number(currentLevel)).toStdString());//key: x_y_level

	//		//current level key
	//		keyList.push_back((QString::number(viewFirstTile.x()  + index0*tileSize) + "_"
	//			+ QString::number(viewFirstTile.y() + index1*tileSize) + "_"
	//			+ QString::number(currentLevel)).toStdString());//key: x_y_level

	//		tileRectList.push_back(temRect);
	//	}
	//}
	//std::swap(oldTileMap, currentTileMap);
	//for (auto _iter = currentTileMap.begin(); _iter != currentTileMap.end(); ++_iter) {
	//	scene()->removeItem(_iter->second);
	//}
	//currentTileMap.clear();
	//for (int index2 = 0; index2 < tileRectList.size(); ++index2) {
	//	creatItemToMap(keyList[index2], tileRectList[index2]);
	//}

	//for (auto _iter = oldTileMap.begin(); _iter != oldTileMap.end(); ++_iter) {
	//	scene()->removeItem(_iter->second);
	//}
	//oldTileMap.clear();
	////auto temList = scene()->items();
	//for (auto _iter = currentTileMap.begin(); _iter != currentTileMap.end(); ++_iter) {
	//	//auto iteFinder1 = currentTileMap.find(*_iter);
	//	QStringList temStr = QString::fromStdString(_iter->first).split("_");
	//	//for (auto iteItem : temList) {
	//	//	auto iteFinder = std::find(temList.begin(), temList.end(), _iter);////////////
	//	//	if (iteFinder != temList.end()) {
	//	//		++iteItem;
	//	//	}
	//	//	else 
	//	//		scene()->addItem(_iter->second);
	//	//}
	//	scene()->addItem(_iter->second);
	//	/*_iter->second->setPos(QPointF(temStr[0].toFloat()*_sceneScale, temStr[1].toFloat()*_sceneScale));*/ //0 level key
	//	_iter->second->setPos(QPointF(temStr[0].toFloat() / upden, temStr[1].toFloat() / upden));//current level key

	//	if (temStr[2].toInt() != 0) {
	//		_iter->second->setZValue(1 / temStr[2].toInt());
	//	}
	//	else _iter->second->setZValue(1);
	//}
}

void Viewer::creatItemToMap(std::string _key, QRectF areaRect)
{
	//if (!oldTileMap.empty()) {
	//	auto iter = oldTileMap.find(_key);
	//	if (oldTileMap.end() != iter) {
	//		currentTileMap[iter->first] = iter->second;//check repeat
	//		oldTileMap.erase(iter);
	//		return;
	//	}
	//}
	//QImage temImg;
	////global coordinate
	//_img->readDataFromImage(areaRect.x(),
	//	areaRect.y(),
	//	areaRect.width(),
	//	areaRect.height(),
	//	currentLevel,
	//	temImg);

	//QPixmap *renderPixmap = new QPixmap; renderPixmap->convertFromImage(temImg);

	////no need for coordinates, just width and height
	//TileGraphicsItem *temTileItem = new TileGraphicsItem(renderPixmap,
	//	areaRect.width(),
	//	areaRect.height(),
	//	areaRect.width() * areaRect.height() * 4,
	//	currentLevel, lastLevel, _parampack->getLevelScaleRate());

	//currentTileMap.insert(std::make_pair(_key, temTileItem));
}
