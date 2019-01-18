#include "Thread2.h"
#include <QReadWriteLock>
#include <QDebug>
#include <QDateTime>


Thread2::Thread2(QObject *parent):
	QThread(parent)
{
	m_flag_stop = true;
	m_flag_exit = false;

	m_time_start = QDateTime::currentDateTime();
}


Thread2::~Thread2()
{
	m_mutex.lock();
	qDebug() << "[Thread2] thread exit: " << thread()->currentThreadId();
	m_mutex.unlock();
}

void Thread2::onPrintMsg()
{
	//m_mutex.lock();
	//m_flag_stop = !m_flag_stop;
	//qDebug() << "[Therad2] Time: " << QDateTime::currentDateTime();
	qDebug() << "[Thread2] onPrintMsg(): " << thread()->currentThreadId();
	//m_mutex.unlock();
}

void Thread2::onChangeFlag()
{
	m_mutex.lock();
	m_flag_stop = !m_flag_stop;
	m_mutex.unlock();
}

void Thread2::onTestFinished()
{
	m_mutex.lock();
	m_flag_exit = !m_flag_exit;
	m_mutex.unlock();
}

void Thread2::run()
{
	while (!m_flag_exit) {
		/*if (!m_flag_stop) {
			onPrintMsg();
		}*/

		//m_time_start = QDateTime::currentDateTime();
		//if (semUsed.tryAcquire()) {
		//	auto tem = que.dequeue();
		//	//qDebug() << "[Thread2] dequeue one: " << tem.first << "Time: " << tem.second;
		//	qDebug() << "[Thread2] total time is: " << tem.second.msecsTo(QDateTime::currentDateTime());
		//	auto tem_diff = m_time_start.msecsTo(QDateTime::currentDateTime());
		//	qDebug() << "[Thread2] tryAcquire time is: " << tem_diff;
		//	m_time_list.push_back(tem_diff);
		//	semFree.release();
		//}

		/*cv::Mat frame = cv::imread("./TF/1826.png");
		cv::imshow("Processor", frame);
		cv::waitKey(2);
		frame.release();*/

		static int index = 0;
		if (semUsed.tryAcquire()) {
			m_frame = csbuffer.dequeue();
			semFree.release();

			if ((nullptr != m_frame) && !m_frame->empty()) {
				cv::imshow("Processor", *(m_frame));
				cv::waitKey(2);

				auto filename = "F:/ForTest/img/" + std::to_string(index++) + ".jpg";
				if (cv::imwrite(filename, *m_frame)) {
					qDebug() << "[Thread2] dequeue one img:" << filename.c_str();
				}
				else {
					qDebug() << "[Thread2] save img failed!";
				}

				//msleep(600);

				m_frame->release();

				qDebug() << "[Thread2] dequeue one img: " << index
					<< ", current avaliabel is: "
					<< semFree.available();
			}
		}
	}
}
