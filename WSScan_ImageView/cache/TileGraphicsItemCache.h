#ifndef TileGraphicsItemCache_H
#define TileGraphicsItemCache_H

#include <QObject>
#include <list>
#include <map>
#include <string>

class TileGraphicsItem;
class Viewer;
class TileManager;


class TileGraphicsItemCache : public QObject {
	Q_OBJECT

public:
	~TileGraphicsItemCache();
	void clear();
	bool GetCache(const std::string&,TileGraphicsItem** tile);
	bool AddCache(const std::string&, int bx, int by, int level, TileGraphicsItem* tile);
	int getSize(){ return m_cache.size(); };//for debug
	//void SortCache();
	void SetViewer(Viewer *arg){ m_viewer = arg; }
	void UpdateCache();
	void SetManager(TileManager *arg);

	struct CacheFlagImage{
		int w = 0, h = 0;
		unsigned char *data_ = 0;
		~CacheFlagImage(){ if (data_) delete[] data_; data_ = 0; w = 0; h = 0; }
	}; // 0 unread 1 reading 2 cache
	std::vector<CacheFlagImage> m_coverageArea;
	std::map<std::string, TileGraphicsItem*> m_cache;

protected:
	void evict();

private:
	Viewer *m_viewer = 0;
	int m_maxCacheNum = 200;
	TileManager *m_manager = 0;


signals:
	void itemEvicted(TileGraphicsItem* item);
};

#endif