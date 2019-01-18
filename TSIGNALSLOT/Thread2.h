#pragma once
#include <vector>
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



class Thread2 : public QThread
{
	Q_OBJECT
public:
	Thread2(QObject *parent = nullptr);
	virtual ~Thread2();

public:
	QMutex m_mutex;

public slots:
	void onPrintMsg();

	void onChangeFlag();

	void onTestFinished();

public:
	void run();

	bool m_flag_stop;
	bool m_flag_exit;

	QDateTime m_time_start;

	std::vector<int> m_time_list;

private:
	cv::Mat* m_frame;
};

