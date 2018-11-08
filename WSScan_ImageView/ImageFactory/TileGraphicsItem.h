#ifndef _TILEGRAPHICSITEM_H
#define _TILEGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPixmap>
#include <ctime>

class TileGraphicsItem : public QGraphicsItem
{
public:
	TileGraphicsItem(QPixmap* item,
		float tileX, float tileY, 
		int bx, int by,
		unsigned int itemLevel, 
		unsigned int lastRenderLevel,
		const std::vector<double>& imgDownsamples);
	virtual ~TileGraphicsItem();

	QPixmap* getPixMap(){ return m_item; }

	virtual QRectF boundingRect() const;

	virtual void paint(QPainter *painter, 
		const QStyleOptionGraphicsItem *option, QWidget *widget);

	/*unsigned int getTileX() { return _tileX; }
	unsigned int getTileY() { return _tileY; }*/
	int getTileLevel() { return m_itemLevel; }
	//unsigned int getTileSize() { return _tileSize; }

	time_t m_usedTime;
	int m_blockX = 0, m_blockY=0;
	float m_xPos=0, m_yPos=0;
	int m_itemLevel;

private:
	QPixmap *m_item = 0;
	float m_physicalSize;
	float m_upperLOD;
	float m_lowerLOD;
	
	//unsigned int _tileX;
	//unsigned int _tileY;
	//unsigned int _tileSize;
	unsigned int m_lastRenderLevel;
	QRectF m_boundingRect;
	QRectF m_drawRect;
	QPointF m_zero;

};

#endif
