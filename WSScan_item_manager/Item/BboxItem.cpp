#include "Item/BboxItem.h"
#include <QPainter>
#include <QPainterPath>
#include <QRectF>
#include <QVector>
#include <QStyleOptionGraphicsItem>
#include <QColor>
#include <QPen>

BboxItem::BboxItem(BboxInfo arg_info, std::string arg_color, QObject* parent) :
	QObject(parent),
	m_info(arg_info),
	m_color(arg_color),
	m_line_thickness(1)
{
	setZValue(3);
}

BboxItem::BboxItem(const BboxItem& arg):
	QObject(arg.parent()),
	m_info(arg.m_info),
	m_color(arg.m_color),
	m_line_thickness(arg.m_line_thickness)
{
	setZValue(3);
}

BboxItem::~BboxItem()
{
}

QPainterPath BboxItem::shape() const
{
	QPainterPath path;
	
	QVector<QPointF> tem_pos;
	tem_pos.push_back(QPointF(-(m_info.width / 2), -(m_info.height / 2)));//top left
	tem_pos.push_back(QPointF(-(m_info.width / 2), m_info.height / 2));//botttom left
	tem_pos.push_back(QPointF(-(m_info.width / 2), -(m_info.height / 2)));//bottom right
	tem_pos.push_back(QPointF(m_info.width / 2, -(m_info.height / 2)));//top right
	tem_pos.push_back(QPointF(-(m_info.width / 2), -(m_info.height / 2)));//top left, for close

	for (int i = 0; i < tem_pos.size(); ++i) {
		path.lineTo(tem_pos[i]);
	}

	//QPolygon polygon;
	//polygon << (QPoint(-(m_info.width / 2), -(m_info.height / 2)));//top left
	//polygon << (QPoint(-(m_info.width / 2), m_info.height / 2));//botttom left
	//polygon << (QPoint(-(m_info.width / 2), -(m_info.height / 2)));//bottom right
	//polygon << (QPoint(m_info.width / 2, -(m_info.height / 2)));//top right
	//polygon << (QPoint(-(m_info.width / 2), -(m_info.height / 2)));//top left, for close
	//path.addPolygon(polygon);

	return path;
}

QRectF BboxItem::boundingRect() const
{
	return QRectF(-(m_info.width / 2), -(m_info.height / 2), m_info.width, m_info.height);
}

void BboxItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
	auto _currentLoD = option->levelOfDetailFromTransform(painter->worldTransform());
	auto tem_pen = painter->pen();

	//such as:   Normal: 0.48
	std::string tem_info = ClassNameMap[m_info.class_id] + ": " + std::to_string(m_info.score);
	QPoint tem_point_text(-(m_info.width / 2) - 5, -(m_info.height / 2) - 5);
	painter->drawText(tem_point_text, tem_info.c_str());

	painter->setPen(QPen(QColor(m_color.c_str()), m_line_thickness / _currentLoD));

	//painter->drawRect(boundingRect());
	//painter->drawPath(shape());//current level

	QVector<QPointF> tem_pos;
	tem_pos.push_back(QPointF(-(m_info.width / 2), -(m_info.height / 2)));//top left
	tem_pos.push_back(QPointF(-(m_info.width / 2), m_info.height / 2));//botttom left
	tem_pos.push_back(QPointF(-(m_info.width / 2), -(m_info.height / 2)));//bottom right
	tem_pos.push_back(QPointF(m_info.width / 2, -(m_info.height / 2)));//top right
	painter->drawRect(QRectF(tem_pos[0].x(), tem_pos[0].y(), m_info.width, m_info.height));
	
}
