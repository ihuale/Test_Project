//#pragma once
//
//#include <list>
//
//#include <QThread>
//#include <QQueue>
//
//#include <opencv2/opencv.hpp>
//#include "utils.h"
//
////ImageCoordinate: for store the id of image
////typedef std::pair<float, float> ImageCoordinate;
//typedef unsigned int ImageID;
//typedef std::pair<cv::Mat, ImageID> ImageMeta;
//
////ResultMeta: <<iamge, id>,<score, class_id>>
//typedef std::pair<ImageID, std::pair<double, int>> ResultMeta;
//
//class TFResultProcessor :
//	public QThread
//{
//	Q_OBJECT
//public:
//	explicit TFResultProcessor(std::list<ResultMeta> *arg_list,
//		QObject *parent = nullptr);
//	~TFResultProcessor();
//
//	//config
//	void setQueueMutex(QMutex& arg_in, QMutex& arg_out);
//	void setWorkState(bool arg);
//	void setSortState(bool arg);
//
//	//function
//	void sort(std::list<ResultMeta> *arg_list);
//	//bool compare(ResultMeta arg_a, ResultMeta arg_b);
//
//	//get result
//	bool getResultList(std::list<ResultMeta>* arg_list);
//
//	void run() override;
//
//private:
//	std::list<ResultMeta> *m_out_list;
//	QMutex *mutex_queue_in, *mutex_queue_out;
//
//	//for thread count
//	static int m_count;
//
//	bool m_flag_work_state;
//	bool m_flag_mutex;
//	bool m_flag_sort;
//};
//
