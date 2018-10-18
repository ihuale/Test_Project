//#include "TFResultProcessor.h"
//#include <algorithm>
//#include <functional>
//
//int TFResultProcessor::m_count = 0;
//
//TFResultProcessor::TFResultProcessor(std::list<ResultMeta> *que,
//	QObject *parent):
//	QThread(parent),
//	m_out_list(que),
//	m_flag_work_state(false),
//	m_flag_mutex(false),
//	m_flag_sort(false)
//{
//	++m_count;
//}
//
//
//TFResultProcessor::~TFResultProcessor()
//{
//	if (m_count > 0)
//		--m_count;
//
//	//prevent accidental exit
//	if (mutex_queue_in)
//		mutex_queue_in->unlock();
//	if (mutex_queue_out)
//		mutex_queue_out->unlock();
//	this->quit();
//}
//
//void TFResultProcessor::setQueueMutex(QMutex & arg_in, QMutex & arg_out)
//{
//	mutex_queue_in = new QMutex;
//	mutex_queue_out = new QMutex;
//	mutex_queue_in = &arg_in;
//	mutex_queue_out = &arg_out;
//	m_flag_mutex = true;
//}
//
//void TFResultProcessor::setWorkState(bool arg)
//{
//	m_flag_work_state = arg;
//}
//
//void TFResultProcessor::setSortState(bool arg)
//{
//	m_flag_sort = arg;
//}
//
//void TFResultProcessor::sort(std::list<ResultMeta>* arg_list)
//{
//	if (arg_list && !arg_list->empty()) {
//		//std::list::sort(arg_list->begin(), arg_list->end(), compare);
//		arg_list->sort(compare);
//	}
//}
//
//bool TFResultProcessor::getResultList(std::list<ResultMeta>* arg_list)
//{
//	if (m_out_list->empty())
//		return false;
//
//	arg_list = m_out_list;
//	return true;
//}
//
//void TFResultProcessor::run()
//{
//	while (m_flag_work_state) {
//
//		mutex_queue_in->lock();
//		if (!m_flag_sort) {
//			mutex_queue_in->unlock();
//		}
//		else {
//			this->sort(m_out_list);
//			m_flag_sort = false;
//			std::cout << "[TFResultProcessor] Sort over!" 
//				<< std::endl;
//		}
//
//	}
//}
