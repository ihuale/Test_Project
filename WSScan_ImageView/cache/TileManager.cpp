#include <vector>
#include <cmath>
#include <omp.h>

#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QRectF>
#include <QPointF>
#include <QGraphicsScene>
#include <QCoreApplication>

#include "TileManager.h"
#include "RenderThread.h"
#include "ImageFactory/OpenSlideImage.h"
#include "ImageFactory/TileGraphicsItem.h"
#include "cache/TileGraphicsItemCache.h"
#include "UI/Viewer.h"

//TileManager::TileManager(OpenSlideImageSP img, 
//	Viewer *viewer, ParampackSPTR param)
TileManager::TileManager(OpenSlideImageSP img,
	Viewer *viewer) :
	m_tileSize(512.)
{
	//m_img = std::make_shared<OpenSlideImage>();
	m_img = img;
	//_parampack = param;//std::make_shared<Parampack>();

	m_thread = new RenderThread(this,4);
	m_thread->setForegroundImage(m_img);

	m_cache = new TileGraphicsItemCache();
	m_cache->SetViewer(viewer);
	m_cache->SetManager(this);

	//_scene = new Viewer;
	m_viewer = viewer;
	m_scene = m_viewer->scene();

	QObject::connect(m_thread, SIGNAL(clearQGraphicsItem()), this, SLOT(onClearQGraphicsItem()));

	std::vector<RenderWorker*> workers = m_thread->getWorkers();
	for (int i = 0; i < workers.size(); ++i) {
		QObject::connect(workers[i], SIGNAL(tileLoaded(QPixmap*, float, float, int, int, int)), this, SLOT(onTileLoaded(QPixmap*, float, float, int, int, int)));
		workers[i]->start(QThread::HighPriority);
	}
}

TileManager::~TileManager()
{
	qDebug() << "TileManager: ~TileManager() 1";
	m_thread = NULL;
	m_cache = NULL;
	m_scene = NULL;
	qDebug() << "TileManager: ~TileManager() 1";
}

void TileManager::loadTilesForFieldOfView(QRectF FOV, int currentLevel)
{
	//the FOV is current level
	//_lastRenderLevel = _parampack->lastRenderLevel;
	m_clearFlag = false;
	//double m_tileSize = _parampack->tileSize;
	double m_tileSize = 512.;
	const int& _lastRenderLevel = m_img->m_lastRenderLevel;
	if (currentLevel > _lastRenderLevel) {
		return;//to be tested
	}

	double den = m_img->getLevelDownsample(currentLevel);
	double upden = std::pow(2, _lastRenderLevel - currentLevel);

	//visibleRect is current view rect
	QRectF visibleCurRect = QRectF(FOV.x() * upden,
		FOV.y() * upden,
		FOV.width() * upden,
		FOV.height() * upden); // cur level

	QPointF temTopLeft = visibleCurRect.topLeft();//cur level
	double tileLevel0 = m_tileSize * den; // level 0 tile size
	int viewTopLeftX = std::floor(temTopLeft.x() / m_tileSize);
	int viewTopLeftY = std::floor(temTopLeft.y() / m_tileSize);
	QPointF viewFirstTile = QPointF((viewTopLeftX)* m_tileSize, (viewTopLeftY)* m_tileSize);//cur level

	int blockX = int(std::floor(visibleCurRect.x() / m_tileSize));
	int blockY = int(std::floor(visibleCurRect.y() / m_tileSize));
	int numOfViewX = std::ceil((visibleCurRect.x() + visibleCurRect.width()) / m_tileSize) - blockX;
	int numOfViewY = std::ceil((visibleCurRect.y() + visibleCurRect.height()) / m_tileSize) - blockY;

	std::list<RenderJob> tmpJobList;
	m_thread->SwapJob(tmpJobList);//clear undone job and update.
	for (auto &it :tmpJobList){
		const int &l = it.m_level;
		const int &bx = it.bx;
		const int &by = it.by;
		m_cache->m_coverageArea[l].data_[bx + by * m_cache->m_coverageArea[l].w] = 0;
	}
	int addNum = 0;
	std::list<RenderJob> jobList;
	RenderJob tmpJob;
	int curBlockX, curBlockY;
	for (int index0 = 0; index0 < numOfViewX; ++index0) {
		for (int index1 = 0; index1 < numOfViewY; ++index1) {
			curBlockX = blockX + index0;
			curBlockY = blockY + index1;
			QPointF temPoint = QPointF(viewFirstTile.x() * den + index0*tileLevel0, viewFirstTile.y() * den + index1*tileLevel0); //global
			TileGraphicsItem* item = 0;
			
			bool temFlag = checkCache(&item, curBlockX, curBlockY, currentLevel);//must be 2
			//has read
			if (temFlag)
			{
				if (m_cache->m_coverageArea[currentLevel].data_[curBlockX + curBlockY * m_cache->m_coverageArea[currentLevel].w] != 2)
				{
					printf("wocao\n");
				}
				if (!item->scene()) {
					m_scene->addItem(item);
					item->m_usedTime = time(0);
				}
			}else if (!temFlag) {
				if (m_cache->m_coverageArea[currentLevel].data_[curBlockX + m_cache->m_coverageArea[currentLevel].w * curBlockY] == 1){//reading
					continue;
				}else
				{
					if (m_cache->m_coverageArea[currentLevel].data_[curBlockX + m_cache->m_coverageArea[currentLevel].w * curBlockY] == 2){
						printf("wolegqu\n");
					}
					m_cache->m_coverageArea[currentLevel].data_[curBlockX + m_cache->m_coverageArea[currentLevel].w * curBlockY] = 1;//reading
					//m_thread->addJob(temPoint.x(), temPoint.y(),curBlockX, curBlockY, currentLevel);
					tmpJob.imgPosX = temPoint.x();
					tmpJob.imgPosY = temPoint.y();
					tmpJob.bx = curBlockX;
					tmpJob.by = curBlockY;
					tmpJob.m_level = currentLevel;
					jobList.push_back(tmpJob);
				}
			}
			else{
				//_scene->addItem(item);
			}
		}
	}
	
	if (!jobList.empty()){
		m_thread->addJobBatch(jobList);
	}
}

void TileManager::refresh()
{
	clear();
	QRectF FOV = m_lastFOV;
	QPointF topLeft = FOV.topLeft();
	QPointF bottomRight = FOV.bottomRight();
	m_lastFOV = QRect();
}

void TileManager::clear()
{
	qDebug() << "TileManager: clear() 0";
	m_thread->clearJobs();
	while (m_thread->getWaitingThreads() != m_thread->getWorkers().size()) {
	}
	QCoreApplication::processEvents();
	if (m_cache) {
		m_cache->clear();
	}
	QList<QGraphicsItem *> itms = m_scene->items();
	for (QList<QGraphicsItem *>::iterator it = itms.begin(); it != itms.end(); ++it) {
		TileGraphicsItem* itm = dynamic_cast<TileGraphicsItem*>((*it));
		if (itm) {
			m_scene->removeItem(itm);
			delete itm;
			//itm = 0;
		}
	}
	qDebug() << "TileManager: clear() 1";
}

void TileManager::onTileLoaded(QPixmap *tile, float tileX, float tileY, int bx, int by, int tileLevel)
{
	TileGraphicsItem *temTileItem = new TileGraphicsItem(tile,
		tileX,
		tileY,
		bx, by,
		tileLevel, 
		m_img->m_lastRenderLevel,
		m_img->getLevelScaleRate());

	double _scale = m_img->getLevelDownsample(tileLevel);
	double upden = std::pow(2, m_img->m_lastRenderLevel - tileLevel);
	auto _currX = tileX / _scale;//cur level
	auto _currY = tileY / _scale;//cur level

	QString temKey = QString::number(bx) + "_"
		+ QString::number(by) + "_"
		+ QString::number(tileLevel);//////!!!!!!!!!!!!!

	if (m_scene) {
		float posX = (_currX / upden);
		float posY = (_currY / upden);
		m_scene->addItem(temTileItem);
		temTileItem->setPos(posX, posY);
		temTileItem->setZValue(1. / ((float)tileLevel + 1.));
	}
	if (m_cache) {
		m_cache->AddCache(temKey.toStdString(), bx, by, tileLevel, temTileItem);
		m_scene->update();
		//printf("cache : %d\n", m_cache->getSize());
	}
	if (m_thread->AllWorkerStop()){
		onClearQGraphicsItem();
	}
}

void TileManager::onTileRemoved(TileGraphicsItem* tile)
{
	m_scene->removeItem(tile);
	//delete tile;
}

bool TileManager::checkCache(TileGraphicsItem** item, int bx, int by, int level)
{
	//arg is global point
	QString temKey = QString::number(bx) + "_"
		+ QString::number(by) + "_"
		+ QString::number(level);

	TileGraphicsItem* temp = 0;
	
	bool flag = m_cache->GetCache(temKey.toStdString(), &temp);
	*item = temp;
	return flag;
}

void TileManager::RemoveCoverage(int x, int y, int level)
{
	//coverageArea_[level][std::pair<int, int>(x, y)] = 0;
}

void TileManager::onClearQGraphicsItem()
{
	for (auto &it : m_cache->m_cache)
	{
		if (!it.second->scene()) continue;
		if (it.second->getTileLevel() != m_img->m_currentLevel) {
			m_scene->removeItem(it.second);
		}
	}

}
