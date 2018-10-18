#pragma once
#include <QGraphicsItem>
#include "head.h"

extern std::string path_image_read;

class BboxItem :public QObject,public QGraphicsItem
{
	Q_OBJECT
public:
	explicit BboxItem(BboxInfo arg_info, 
		std::string arg_color, QObject *parent = Q_NULLPTR);
	explicit BboxItem(const BboxItem &arg);
	~BboxItem();

	virtual QPainterPath shape() const;
	virtual QRectF boundingRect() const;
	virtual void paint(QPainter *painter,
		const QStyleOptionGraphicsItem *option, QWidget *widget);

public:
	BboxInfo m_info;
	std::string m_color;
	float m_line_thickness;

};