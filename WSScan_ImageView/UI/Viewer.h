#ifndef _Viewer_H
#define _Viewer_H

#include <memory>
#include <QGraphicsView>
#include <QSettings>
#include <QTimeLine>
#include <QTimer>
#include <QPoint>
#include <memory>
//#include "ImageFactory/Parampack.h"
#include "ImageFactory/OpenSlideImage.h"

class TileGraphicsItem;
class TileGraphicsItemCache;
class RenderThread;
class TileManager;
class BaseManager;

class Viewer:public QGraphicsView
{
	Q_OBJECT
public:
	Viewer(QWidget *parent = 0);
	~Viewer();

	QPointF _zoomToScenePos;
	QPointF _zoomToViewPos;

	void initialize(OpenSlideImageSP img);
	void close();

	void getBoundingPoint();
	void calcVisibleLevel();

	QRectF& GetFOV(){ return FOV_; }

signals:
	void parentShowCoordinate(std::string arg ,int _time);
	void viewOfFieldChanged();
	void deleteItem();

private slots:
	void scalingTime(qreal x);
	void zoomFinished();
	void StopZoomTimer();
	void onViewOfFieldChanged();

private:
	//should store all manager to stl   !!!!!!!!!!!!!!
	std::shared_ptr<BaseManager> _activemanager = 0;

	std::map<std::string, TileGraphicsItem*> currentTileMap;
	std::map<std::string, TileGraphicsItem*> oldTileMap;
	std::vector<double> ratio;

	TileGraphicsItem *temItem = 0;//for show when initialize
	OpenSlideImageSP m_img;
	TileManager *m_tileManager = 0;//this manager do not changed

	QSettings *_settings = 0;
	QPoint _prevPan;
	QRectF globalImgRect_;
	QRectF visibleRect_;
	QTimer *zoomTimer = 0;
	QRectF FOV_;

	unsigned int zoomTime = 100;
	unsigned int lastLevel;

	double _sceneScale;

	float _numScheduledScalings;
	float totalScale;
	float currentLevelScale;


	int numSteps;
	int oldLevel;

	//for state
	bool isPanning;

	unsigned char mouseMoveTriggerFlag_ = 1;

	virtual void wheelEvent(QWheelEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void resizeEvent(QResizeEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);

	void pan(QPoint);
	void zoom(float);
	void togglePan(bool pan, const QPoint& startPos = QPoint());
	void readNewFOI();
	
	void splitImage();
	void creatItemToMap(std::string, QRectF);
};

#endif
