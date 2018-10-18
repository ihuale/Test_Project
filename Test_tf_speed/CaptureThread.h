#pragma once
#include <QThread>
#include <QObject>
#include "utils.h"

class CaptureThread :
	public QThread
{
public:
	explicit CaptureThread(QQueue<ImageMeta>* arg_inque,
		QMutex* arg_mutex,
		QObject* parent = nullptr);
	virtual ~CaptureThread();

	void run();

	void setPiece(int arg_x, int arg_y);
	void setBatchInfo(
		int arg_batch_size,
		int arg_width,
		int arg_height,
		int arg_channels);

private:
	QQueue<ImageMeta> *m_inQue;
	QMutex* m_mutex;

	int m_piece_x;
	int m_piece_y;

	int m_batch_size;
	int m_image_width;
	int m_image_height;
	int m_image_channels;
};

