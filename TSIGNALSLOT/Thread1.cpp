#include "Thread1.h"
#include <QReadWriteLock>
#include <QDebug>
#include <QDateTime>


Thread1::Thread1(QObject *parent):
	QThread(parent)
{
}


Thread1::~Thread1()
{
	m_mutex.lock();
	qDebug() << "[Thread1] thread exit: " << thread()->currentThreadId();
	m_mutex.unlock();
}

void Thread1::run()
{
	int num = semFree.available();
	cv::VideoCapture cap(0);

	int index = 0;
	while(cap.isOpened()){
	//while (num--) {
		//emit PrintMsg();
		//m_mutex.lock();
		//qDebug() << "[Therad1] Time: " << QDateTime::currentDateTime();
		//qDebug() << "[Thread1] thread id: " << thread()->currentThreadId();
		//m_mutex.unlock();

		//semFree.acquire();
		//std::pair<int, QDateTime> tem;
		//tem.first = num;
		//tem.second = QDateTime::currentDateTime();
		//que.enqueue(tem);
		///*sleep(0.1);*/
		////qDebug() << "[Thread1] enqueue one: " << tem.first << "Time: " << tem.second;
		//semUsed.release();
		if (!m_frame.empty()) {
			m_frame.release();
		}

		cap >> m_frame;

		cv::putText(m_frame,
			std::to_string(index++).substr(0, 5),
			cv::Point(0, m_frame.rows),
			cv::FONT_HERSHEY_SIMPLEX,
			0.7,
			cv::Scalar(255, 0, 0));
		cv::imshow("Capture", m_frame);
		cv::waitKey(2);

		semFree.acquire();
		if (csbuffer.enqueue(m_frame.clone())) {
			qDebug() << "[Thread1] enqueue one img!";
		}
		else {
			qDebug() << "[Thread1] enqueue one img failed!";
		}
		semUsed.release();
	}
}
