#include "TileMap.h"
#include <QKeyEvent>
#include <QPainter>
#include <QVector>
#include <QPoint>
#include <QPen>
#include <QTimer>
#include <QDateTime>

#include <iostream>



TileMap::TileMap(QWidget *parent,
	int arg_piece_x,
	int arg_piece_y) :
	QLabel(parent),
	piece_x(arg_piece_x),
	piece_y(arg_piece_y),
	m_direction(Direction::RowFirst),
	diff_x(10),
	diff_y(5),
	m_img_current(0)
{
	m_pixmap_init = new QPixmap(":/Test_tf_speed/Resources/Init.bmp");
	m_pixmap_Identified = new QPixmap(":/Test_tf_speed/Resources/Identified.bmp");
	m_pixmap_Scanned = new QPixmap(":/Test_tf_speed/Resources/Current.bmp");

	m_rect_anchor = new QRect;
	this->setFixedSize(piece_x*diff_x + 2, piece_y*diff_y + 2);
	//first paint,only draw the split lines
	this->update();

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(test_timer_add_result()));
	//timer->start(10);
}


TileMap::~TileMap()
{
	if (m_list_map.size() > 1) {
		m_list_map.clear();
	}
}

void TileMap::setDirection(Direction arg)
{
	if (!arg) {
		return;
	}
	m_direction = arg;
}

bool TileMap::get_tile_coordinate( ImageID arg, QRect* arg_res)
{
	//TODO
	//check first
	auto tem_finder = m_list_map.find(arg);
	if (tem_finder == m_list_map.end()) {
		//not found
		return false;
	}
	int tem_piece_x = 0, tem_piece_y = 0;
	if (m_direction == RowFirst) {
		tem_piece_x = arg % piece_x;
		tem_piece_y = std::floor(arg / piece_x);
	}
	else if (m_direction == ColFirst) {
		tem_piece_x = std::floor(arg / piece_y);
		tem_piece_y = arg % piece_y;
	}

	*arg_res = QRect(tem_piece_x*diff_x+1, 
		tem_piece_y*diff_y+1, 
		diff_x,
		diff_y);

	return true;
}

void TileMap::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	auto tem_pen = painter.pen();//for restore
	tem_pen.setBrush(QBrush(Qt::white));

	//first,draw the tile, and cloring
	//tem_rect for store the coordinate of the tile
	QRect tem_rect = QRect(0, 0, 0, 0);
	for (auto &tem_iter : m_list_map) {
		auto tem_falg_get = 
			get_tile_coordinate(tem_iter.first, &tem_rect);
		if (!tem_falg_get) {
			break;
		}

		//according to score,draw it
		if (tem_iter.second.first > 50) {
			//painter.setPen(Qt::red);
			painter.fillRect(tem_rect, Qt::red);
			//painter.drawPixmap(tem_rect.x(), tem_rect.y(),
				//m_pixmap_Identified->scaled(10, 5));
		}
		else {
			//painter.setPen(Qt::green);
			painter.fillRect(tem_rect, Qt::green);
			//painter.drawPixmap(tem_rect.x(), tem_rect.y(), 
				//m_pixmap_Scanned->scaled(10, 5));
		}
	}

	//then,draw anchor for current img piece
	if (m_list_map.size() > 2 && m_rect_anchor) {
		QPen pen;
		pen.setBrush(QBrush(Qt::blue));//set brush
		QVector<qreal> dashes;
		qreal space = 2;
		dashes << 4 << space << 4 << space;
		pen.setDashPattern(dashes);
		pen.setWidth(1);
		painter.setPen(pen);
		painter.fillRect(
			m_rect_anchor->center().x() - diff_x,
			m_rect_anchor->center().y() - diff_y,
			diff_x * 2,
			diff_y * 2,
			Qt::blue);
		/*painter.drawRect(
			m_rect_anchor->center().x()-diff_x,
			m_rect_anchor->center().y()-diff_y,
			diff_x*2,
			diff_y*2);*/
		painter.setPen(tem_pen);
	}

	//last,draw the line
	//piece_y default:20
	for (int i_y = 0, tem_y = 1; i_y <= piece_y; ++i_y, tem_y += diff_y) {
		painter.drawLine(1, tem_y, piece_x*diff_x + 1, tem_y);
	}
	//piece_x default:30
	for (int i_x = 0, tem_x = 1; i_x <= piece_x; ++i_x, tem_x += diff_x) {
		painter.drawLine(tem_x, 1, tem_x, piece_y*diff_y + 1);
	}
}

void TileMap::keyPressEvent(QKeyEvent* event)
{
	if (!this->isEnabled() || m_list_map.size() < 1) {
		return;
	}

	//key event
	switch (event->key())
	{
	case Qt::Key_Left: {
		if (m_direction == RowFirst) {
			move_to_tile(m_img_current, -1);
		}
		else if (m_direction == ColFirst) {
			move_to_tile(m_img_current, -piece_y);
		}
		break;
	}
	case Qt::Key_Right: {
		if (m_direction == RowFirst) {
			move_to_tile(m_img_current, 1);
		}
		else if (m_direction == ColFirst) {
			move_to_tile(m_img_current, piece_y);
		}
		break;
	}
	case Qt::Key_Up: {
		if (m_direction == RowFirst) {
			move_to_tile(m_img_current, -piece_x);
		}
		else if (m_direction == ColFirst) {
			move_to_tile(m_img_current, -1);
		}
		break;
	}
	case Qt::Key_Down: {
		if (m_direction == RowFirst) {
			move_to_tile(m_img_current, piece_x);
		}
		else if (m_direction == ColFirst) {
			move_to_tile(m_img_current, 1);
		}
		break;
	}
	case Qt::Key_R: {
		//this just for test
		int tem_image_id = rand() % (piece_x*piece_y);
		int tem_image_flag = (rand() % 2) ? -1 : 1;
		int tem_image_diff = (rand() % 30) * tem_image_flag;
		move_to_tile(tem_image_id, tem_image_diff);
	}
	default:
		break;
	}
}

bool TileMap::move_to_tile(ImageID arg)
{
	if (m_list_map.size() < 2) {
		return false;
	}
	//TODO
	QRect tem_rect = QRect(0, 0, 0, 0);
	auto flag_get_tile =
		get_tile_coordinate(arg, &tem_rect);
	if (!flag_get_tile) {
		//not found
		return false;
	}
	*m_rect_anchor = tem_rect;
	m_img_current = arg;
	update();
	return true;
}

bool TileMap::move_to_tile(ImageID arg, int arg_diff)
{
	std::cout << "StateLabel::move_to_tile:\nImageID:" 
		<< arg 
		<< "   diff:" 
		<< arg_diff 
		<< std::endl;
	return move_to_tile(arg + arg_diff);
}

void TileMap::on_signal_result(ResultMeta arg)
{
	std::cout << "[TileMap] add_result:"
		<< arg.first
		<< "--" << arg.second.first
		<< "--" << arg.second.first
		<< std::endl;
	add_result(arg);
}

void TileMap::resetColAndRow(int arg_row, int arg_col)
{
	m_list_map.clear();
	piece_x = arg_row;
	piece_y = arg_col;
	this->update();
}

void TileMap::test_timer_add_result()
{
	static int i = 0;
	if (m_list_map.size() >= piece_x*piece_y) {
		/*timer->stop();
		return;*/
		//for test, haha
		m_list_map.clear();
		i = 0;
	}
	this->add_result(std::make_pair(i++,
		std::make_pair(rand() % 100,
			rand() % 3)));
}

void TileMap::add_result(ResultMeta arg)
{
	/*if (arg) {
		return;
	}*/
	std::cout << "[TileMap] add_result:"
		<< arg.first
		<< "--" << arg.second.first
		<< "--" << arg.second.first
		<< std::endl;
	static int tile_count = 0;
	if (tile_count >= (piece_x*piece_y)) {
		tile_count = 0;
		resetColAndRow(piece_x, piece_y);
	}
	m_list_map[arg.first] = arg.second;
	move_to_tile(arg.first);
	//m_img_current = arg.first;
	this->update();
	++tile_count;
}