#include "TileGraphicsItemCache.h"
#include "ImageFactory/TileGraphicsItem.h"
#include "UI/Viewer.h"
#include "TileManager.h"
//#include "ImageFactory/Parampack.h"
#include <QDebug>

TileGraphicsItemCache::~TileGraphicsItemCache() {
	clear();
}

void TileGraphicsItemCache::evict() {
	if (m_cache.empty()) return;
	std::map<std::string, TileGraphicsItem*>::iterator removeIt = m_cache.begin();
	time_t lastTime = m_cache.begin()->second->m_usedTime;
	for (auto iter = m_cache.begin(); iter != m_cache.end(); ++ iter) {
		if (iter->second->m_usedTime < lastTime) {
			removeIt = iter;
			lastTime = iter->second->m_usedTime;
		}
	}
	
	if (removeIt->second->scene()){
		removeIt->second->scene()->removeItem(removeIt->second);
	}
	m_coverageArea[removeIt->second->m_itemLevel].data_[removeIt->second->m_blockX + removeIt->second->m_blockY * m_coverageArea[removeIt->second->m_itemLevel].w] = 0;
	delete removeIt->second;
	removeIt->second = NULL;
	m_cache.erase(removeIt);
	
}

void TileGraphicsItemCache::clear() {
	if (!m_cache.empty()) {
		m_cache.clear();
	}
}

bool TileGraphicsItemCache::GetCache(const std::string& str, TileGraphicsItem** tile)
{
	std::map<std::string, TileGraphicsItem*>::iterator it = m_cache.find(str);
	if (it != m_cache.end()) {
		*tile = it->second;
		it->second->m_usedTime = time(0);
		return true;
	}
	return false;
}

bool TileGraphicsItemCache::AddCache(const std::string& str, int bx, int by, int level, TileGraphicsItem* tile)
{
	std::map<std::string, TileGraphicsItem*>::iterator it = m_cache.find(str);
	if (it != m_cache.end()) {
		qDebug() << it->first.c_str();
		return false;
	}
	else{
		tile->m_usedTime = time(0);
		m_cache.insert(std::make_pair(str, tile));
		m_coverageArea[level].data_[bx + m_coverageArea[level].w * by] = 2;
		UpdateCache();
		return true;
	}
}

void TileGraphicsItemCache::UpdateCache()
{
	int diff = m_cache.size() - m_maxCacheNum;
	if (diff < 1) return;
	for (int i = 0; i < diff; ++i){
		evict();
	}
}

void TileGraphicsItemCache::SetManager(TileManager *arg)
{
	m_manager = arg;
	m_coverageArea.resize(m_manager->m_img->m_lastRenderLevel + 1);
	for (int i = 0; i <= m_manager->m_img->m_lastRenderLevel; ++i){
		int xNum = int(std::ceil(float(arg->m_img->m_dims[i][0]) / float(m_manager->m_tileSize)));
		int yNum = int(std::ceil(float(arg->m_img->m_dims[i][1]) / float(m_manager->m_tileSize)));
		m_coverageArea[i].data_ = new unsigned char[xNum * yNum];
		m_coverageArea[i].w = xNum;
		m_coverageArea[i].h = yNum;
		memset(m_coverageArea[i].data_, 0, xNum * yNum);
	}
}
