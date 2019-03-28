#pragma once
#include <string>

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QQueue>

#include "AIType.h"

extern std::string path_image_read;
extern std::string image_filter;

class ImageReadThread :public QThread
{
	Q_OBJECT
public:
	explicit ImageReadThread(QQueue<ImageCvMeta>* arg_out_list,
		QMutex *arg_mutex,
		QObject *parent=nullptr);
	virtual ~ImageReadThread();

	void setPath(std::string arg_path);

	void run();

private:
	std::string m_read_path;

	QQueue<ImageCvMeta>* m_out_queue;

	QMutex *m_mutex;
};