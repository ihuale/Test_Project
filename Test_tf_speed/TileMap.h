#pragma once
#include <QLabel>
#include <QRect>
#include <utility>
#include <map>

#include <opencv/highgui.h>

typedef unsigned int ImageID;
typedef std::pair<cv::Mat, ImageID> ImageMeta;

//ResultMeta: <<iamge, id>,<score, class_id>>
//ScoreMeta: <score, class_id>
typedef std::pair<double, int> ScoreMeta;
typedef std::pair<ImageID, std::pair<double,int>> ResultMeta;
//typedef std::pair<ImageID, ScoreMeta> ResultMeta;
typedef std::map<ImageID, ScoreMeta> ResultMap;


class TileMap : public QLabel
{
	Q_OBJECT
public:
	explicit TileMap(QWidget *parent=nullptr,
		int arg_piece_x = 20,
		int arg_piece_y = 30);
	virtual ~TileMap();

	enum Direction{
		RowFirst,
		ColFirst
	};

	//config
	void setDirection(Direction arg);

	//function
	bool get_tile_coordinate(
		ImageID arg,
		QRect* arg_res);

	virtual void paintEvent(QPaintEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);

public slots:
	void add_result(ResultMeta arg);
	bool move_to_tile(ImageID arg);
	bool move_to_tile(ImageID arg, int arg_diff);

	//for receive result
	void on_signal_result(ResultMeta arg);

	//
	void resetColAndRow(int arg_row, int arg_col);

	//for test
	void test_timer_add_result();

private:
	ResultMap m_list_map;

	QPixmap *m_pixmap_init;
	QPixmap *m_pixmap_Identified;
	QPixmap *m_pixmap_Scanned;

	//the piece count of x and y
	int piece_x;
	int piece_y;
	int diff_x;
	int diff_y;

	//move value
	ImageID m_img_current;
	QRect* m_rect_anchor;

	//the direction of iteration
	Direction m_direction;

	//for test
	QTimer *timer;
};

