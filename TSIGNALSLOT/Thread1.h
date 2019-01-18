#pragma once
#include <utility>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QDateTime>
#include <QSemaphore>
#include "CSBuffer.hpp"

extern QSemaphore semFree;
extern QSemaphore semUsed;
extern QQueue<std::pair<int, QDateTime>> que;
extern CSBuffer csbuffer;


class Thread1 :
	public QThread
{
	Q_OBJECT
public:
	Thread1(QObject *parent = nullptr);
	virtual ~Thread1();

signals:
	void PrintMsg();

public:
	void run();

public:
	QMutex m_mutex;

private:
	cv::Mat m_frame;
};

