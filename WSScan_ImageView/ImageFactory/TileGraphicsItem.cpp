#include "TileGraphicsItem.h"
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QDebug>

TileGraphicsItem::TileGraphicsItem(QPixmap* item, 
	float tileX, float tileY, 
	int bx, int by,
	unsigned int itemLevel, unsigned int lastRenderLevel, 
	const std::vector<double>& imgDownsamples) :
	QGraphicsItem(),
	m_item(item),
	m_blockX(bx), m_blockY(by),
	m_itemLevel(itemLevel),
	m_lastRenderLevel(lastRenderLevel)
{
	//_physicalSize = _tileSize / (imgDownsamples[_lastRenderLevel] / imgDownsamples[_itemLevel]);
	float lastRenderLevelDownsample = imgDownsamples[m_lastRenderLevel];
	float itemLevelLOD = lastRenderLevelDownsample / imgDownsamples[m_itemLevel];
	if (m_itemLevel == m_lastRenderLevel) {
		m_lowerLOD = 0.;
	}
	else {
		float avgDownsample = (imgDownsamples[m_itemLevel + 1] + imgDownsamples[m_itemLevel]) / 2.;
		m_lowerLOD = lastRenderLevelDownsample / avgDownsample;
	}
	if (m_itemLevel == 0) {
		m_upperLOD = std::numeric_limits<float>::max();
	}
	else {
		float avgDownsample = (imgDownsamples[m_itemLevel - 1] + imgDownsamples[m_itemLevel]) / 2.;
		m_upperLOD = lastRenderLevelDownsample / avgDownsample;
	}
	this->setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
	if (itemLevel != 0) {
		this->setZValue(1 / itemLevel);
	}
	else this->setZValue(1);
	
	m_boundingRect.setRect(0, 0, m_item->width(), m_item->height());
	//_boundingRect = QRectF(-_physicalSize / 2., -_physicalSize / 2., _physicalSize, _physicalSize);
	m_zero.setY(0);
	m_zero.setX(0);
	m_xPos = tileX;
	m_yPos = tileY;
}

TileGraphicsItem::~TileGraphicsItem()
{
	if (m_item) {
		delete m_item;
		m_item = NULL;
	}
}

QRectF TileGraphicsItem::boundingRect() const{
	return this->m_boundingRect;
}

void TileGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
	if (true) {
		if (m_item->width()>10) {
			m_drawRect.setTopLeft(QPointF(0., 0.));
			m_drawRect.setBottomRight(QPointF(m_item->width() / std::pow(2, m_lastRenderLevel - m_itemLevel), m_item->height() / std::pow(2, m_lastRenderLevel - m_itemLevel)));
			painter->drawPixmap(m_drawRect, *m_item, m_item->rect());
			//painter->drawRect(drawRect_);
			m_boundingRect.setRect(0, 0, m_drawRect.width(), m_drawRect.height());
		}
	}
}