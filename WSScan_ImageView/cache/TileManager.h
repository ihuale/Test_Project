#pragma once

#include <memory>
#include <QString>
#include <QObject>
#include <QPixmap>
#include <QGraphicsScene>
#include "ImageFactory/OpenSlideImage.h"
//#include "ImageFactory/Parampack.h"
#include "TileGraphicsItemCache.h"

class RenderThread;
class RenderWorker;
class Viewer;
class TileGraphicsItem;


class TileManager : public QObject
{
	Q_OBJECT

public:
	TileManager(OpenSlideImageSP img, 
		Viewer *viewer);
	~TileManager();

	//void setParampack(ParampackSPTR arg){ _parampack = arg; };
	void loadTilesForFieldOfView(QRectF FOV,int level);//current level rect
	void refresh();
	void clear();
	void RemoveCoverage(int, int, int);

	//ParampackSPTR _parampack;
	double m_tileSize;
	//int _lastRenderLevel;

	QRectF m_lastFOV;

	OpenSlideImageSP m_img;

	RenderThread *m_thread = 0;
	TileGraphicsItemCache *m_cache = 0;
	Viewer *m_viewer = 0;
	QGraphicsScene *m_scene = 0;


public slots:
	void onTileLoaded(QPixmap *tile, float tileX, float tileY, int bx, int by, int tileLevel);

	void onTileRemoved(TileGraphicsItem* tile);
	void onClearQGraphicsItem();

private:
	
	bool m_clearFlag = false;

	//MPITileGraphicsItem *backgroundItem_;// never delete

	bool checkCache(TileGraphicsItem**, int bx, int by, int level);
};
