//#include <QtCore/QCoreApplication>
//
//#define COMPILER_MSVC
//#define NOMINMAX
//
//#include <stdio.h>
//#include <tchar.h>
//
//#include <numeric>
//#include <eigen/Dense>
//#include <fstream>
//#include <utility>
//#include <vector>
//#include <fstream>
//#include <iostream>
//#include <regex>
//
//#include "tensorflow/cc/ops/const_op.h"
//#include "tensorflow/cc/ops/image_ops.h"
//#include "tensorflow/cc/ops/standard_ops.h"
//#include "tensorflow/core/framework/graph.pb.h"
//#include "tensorflow/core/framework/tensor.h"
//#include "tensorflow/core/graph/default_device.h"
//#include "tensorflow/core/graph/graph_def_builder.h"
//#include "tensorflow/core/lib/core/errors.h"
//#include "tensorflow/core/lib/core/stringpiece.h"
//#include "tensorflow/core/lib/core/threadpool.h"
//#include "tensorflow/core/lib/io/path.h"
//#include "tensorflow/core/lib/strings/str_util.h"
//#include "tensorflow/core/lib/strings/stringprintf.h"
//#include "tensorflow/core/platform/env.h"
//#include "tensorflow/core/platform/init_main.h"
//#include "tensorflow/core/platform/logging.h"
//#include "tensorflow/core/platform/types.h"
//#include "tensorflow/core/public/session.h"
//#include "tensorflow/core/util/command_line_flags.h"
//
//#include <opencv2/opencv.hpp>
//
//#include <QQueue>
//#include <QMutex>
//#include <QVector>
//#include <QThread>
//#include <QRunnable>
//#include <QThreadPool>
//#include <QObject>
//
//#include "CheckOutputQue.h"
//#include "Processor.h"
//#include "Processor_2.h"
//#include "Processor_3.h"
//#include "utils.h"
//
//
//// These are all common classes it's handy to reference with no namespace.
//using tensorflow::Flag;
//using tensorflow::Tensor;
//using tensorflow::Status;
//using tensorflow::string;
//using tensorflow::int32;
//
//
//
//bool g_stop = false;
//QMutex g_mutex_in, g_mutex_out;
//
//
//int main(int argc, char *argv[])
//{
//	QCoreApplication a(argc, argv);
//
//
//	cv::Mat frame;
//	Tensor tensor;
//	std::vector<Tensor> outputs;
//	double thresholdScore = 0.5;
//	double thresholdIOU = 0.8;
//
//	// FPS count
//	int nFrames = 32;
//	int iFrame = 0;
//	double fps = 0.;
//	time_t start, end;
//	time(&start);
//
//	cv::VideoCapture cap(0);
//
//	tensorflow::TensorShape shape = tensorflow::TensorShape();
//	shape.AddDim(1);
//	shape.AddDim((int64)cap.get(cv::CAP_PROP_FRAME_HEIGHT));
//	shape.AddDim((int64)cap.get(cv::CAP_PROP_FRAME_WIDTH));
//	shape.AddDim(3);
//
//	QQueue<cv::Mat> inQue, outQue;
//	QVector<Processor *> processWorkers;
//	QVector<Processor_2 *> process_2Workers;
//	QVector<Processor_3 *> process_3Workers;
//
//	int processor_flag = 1;
//	int numWorkers = 14;
//	switch (processor_flag)
//	{
//	case 0:
//	{
//		for (int i = 0; i != numWorkers; i++) {
//			processWorkers.append(new Processor());
//			processWorkers[i]->setParams(&inQue, &outQue);
//			processWorkers[i]->setSession();
//			processWorkers[i]->start();
//			//QThreadPool::globalInstance()->start(processWorkers[i]);
//		}
//		break;
//	}
//	case 1:
//	{
//		for (int i = 0; i != numWorkers; i++) {
//			process_2Workers.append(new Processor_2());
//			process_2Workers[i]->setParams(&inQue, &outQue);
//			process_2Workers[i]->setSession();
//			process_2Workers[i]->start();
//			//QThreadPool::globalInstance()->start(processWorkers[i]);
//		}
//		break;
//	}
//	case 2:
//	{
//		for (int i = 0; i != numWorkers; i++) {
//			process_3Workers.append(new Processor_3());
//			process_3Workers[i]->setParams(&inQue, &outQue);
//			process_3Workers[i]->setSession();
//			process_3Workers[i]->start();
//			//QThreadPool::globalInstance()->start(processWorkers[i]);
//		}
//		break;
//	}
//	default:
//		std::cout << "Wrong processor_flag!" << std::endl;
//		return -1;
//	}
//
//	//QMutex mutex_local_main;
//
//	CheckOutputQue* checkWorker = new CheckOutputQue(&outQue);
//	checkWorker->start();
//
//	int max_num_frame = 1500;
//	/*cv::Mat frame_local = cv::imread("./TF/5027.png");
//	double tem_mat_min = 0., tem_mat_max = 0.;
//	cv::minMaxIdx(frame_local, &tem_mat_min, &tem_mat_max);
//	auto tem_mat_mid = (tem_mat_max - tem_mat_max) / 2.;
//	frame = (frame_local - tem_mat_min) / (tem_mat_max - tem_mat_min);*/
//	sleep(5);
//
//	while (cap.isOpened()) {
//		cap >> frame;//for normlize
//		cv::imshow("in", frame);
//		/*double tem_mat_min = 0., tem_mat_max = 0.;
//		cv::minMaxIdx(frame, &tem_mat_min, &tem_mat_max);
//		auto tem_mat_mid = (tem_mat_max - tem_mat_max) / 2.;
//		frame = (frame - tem_mat_min) / (tem_mat_max - tem_mat_min);*/
//
//		cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
//		std::cout << "frame # " << iFrame << std::endl;
//
//
//		//mutex_local_main.lock();
//		if (nFrames % (iFrame + 1) == 0) {
//			//time(&end);
//			end = clock();
//			fps = nFrames / difftime(end, start) * 1000.;
//			//time(&start);
//			start = clock();
//		}
//		iFrame++;
//		//mutex_local_main.unlock();
//		cv::Mat tem_frame(512, 512, CV_32FC3);//for processor_2
//		//cv::Mat tem_frame(299, 299, CV_32FC3);//for processor_3
//		//if (iFrame < max_num_frame) {
//			g_mutex_in.lock();
//			cv::resize(frame, tem_frame, cv::Size(512, 512));//for processor_2
//			//cv::resize(frame, tem_frame, cv::Size(299, 299));//for processor_3
//			//inQue.enqueue(frame.clone());
//			inQue.enqueue(tem_frame.clone());//for processor_2 and processor_3
//			//std::cout << "[Thread] enqueueing" << std::endl;
//			g_mutex_in.unlock();
//		//}
//
//		
//		//for processor_2
//		cv::putText(tem_frame,
//			std::to_string(fps).substr(0, 5),
//			cv::Point(0, tem_frame.rows),
//			cv::FONT_HERSHEY_SIMPLEX,
//			0.7,
//			cv::Scalar(255, 0, 0));
//
//		cv::cvtColor(tem_frame, tem_frame, cv::COLOR_BGR2RGB);
//		cv::imshow("stream", tem_frame);
//		cv::waitKey(5);
//		//for processor_2
//		
//
//		/*
//		//for processor
//		cv::putText(frame,
//		std::to_string(fps).substr(0, 5),
//		cv::Point(0, frame.rows),
//		cv::FONT_HERSHEY_SIMPLEX,
//		0.7,
//		cv::Scalar(255, 0, 0));
//
//		cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
//		cv::imshow("stream", frame);
//		cv::waitKey(5);
//		//for processor
//		*/
//	}
//
//	cv::destroyAllWindows();
//
//	getchar();
//
//	a.quit();
//
//	return a.exec();
//}
