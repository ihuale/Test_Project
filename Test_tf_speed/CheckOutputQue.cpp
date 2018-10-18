#include "CheckOutputQue.h"
#include <QMutex>

extern bool g_stop;
extern QMutex g_mutex_out;

CheckOutputQue::CheckOutputQue(QQueue<ResultMeta> *arg_que,
	QObject *parent)
	: QThread(parent),
	m_outQue(arg_que),
	m_flag_queue(true),
	m_flag_list(false)
{
}

CheckOutputQue::CheckOutputQue(std::list<ResultMeta>* arg_list, 
	QObject * parent)
	:QThread(parent),
	m_out_list(arg_list),
	m_flag_queue(false),
	m_flag_list(true)
{
}

CheckOutputQue::CheckOutputQue(ResultMap * map, 
	QObject * parent):
	QThread(parent),
	m_map(map),
	m_flag_queue(false),
	m_flag_list(false),
	m_flag_map(true)
{
}

CheckOutputQue::~CheckOutputQue()
{
}

void CheckOutputQue::run()
{
	int nFrames = 32;
	int iFrame = 0;
	double fps = 0.;
	time_t start, end;
	time(&start);

	std::cout << "[Thread] waiting for check output que" << std::endl;

	while (!g_stop) {

		g_mutex_out.lock();
		if ((m_flag_queue && m_outQue->isEmpty())
			|| (m_flag_list && m_out_list->empty())
			|| (m_flag_map && m_map->empty())) {
			g_mutex_out.unlock();
			//std::cout << "[Thread] looping inside checkoutput que" << std::endl;
		}
		else
		{
			ResultMeta frame;
			if (m_flag_queue) {
				frame = m_outQue->dequeue();
			}
			else if (m_flag_list) {
				frame = m_out_list->back();
			}
			else if (m_flag_map) {
				frame = (*m_map->begin());
			}

			g_mutex_out.unlock();



			if ((iFrame + 1) % nFrames == 0) {
				//time(&end);
				end = clock();
				std::cout << "[ChenkOutQueue] difftime(): " << difftime(end, start) << std::endl;
				//fps = 1. * nFrames / difftime(end, start);
				fps = nFrames / ((difftime(end, start)) / 1000.);
				//time(&start);
				start = clock();
			}
			iFrame++;
			
			/*cv::putText(frame.first.first, 
				std::to_string(fps).substr(0, 5),
				cv::Point(0, frame.first.first.rows),
				cv::FONT_HERSHEY_SIMPLEX,
				0.7,
				cv::Scalar(255, 0, 0));
			cv::imshow("output", frame.first.first);*/
			if (iFrame % 32 == 0) {
				std::cout << "[CheckOutputQue]  fps is :" << fps
					<< "*********************************************"
					<< std::endl;
			}
			cv::waitKey(5);
		}
	}


}

