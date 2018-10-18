//#pragma once
//#ifndef COMPILER_MSVC
//#define COMPILER_MSVC
//#endif //COMPILER_MSVC
//
//#ifndef NOMINMAX
//#define NOMINMAX
//#endif //NOMINMAX
//
//#include <numeric>
//#include <eigen/Dense>
//#include <fstream>
//#include <utility>
//#include <vector>
//#include <string>
//#include <fstream>
//#include <iostream>
//#include <regex>
//
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
//#include "tensorflow/core/platform/types.h"
//#include "tensorflow/core/public/session.h"
//#include "tensorflow/core/public/session_options.h"
//
//#include  <opencv2/opencv.hpp>
//
//#include <QObject>
//#include <QThread>
//#include <QQueue>
//#include <QMutex>
//
//class Processor_2 :
//	public QThread
//{
//	Q_OBJECT
//public:
//	explicit Processor_2(QObject *pParent = nullptr);
//	virtual ~Processor_2();
//	void setSession();
//	void setParams(QQueue<cv::Mat> *inQue,
//		QQueue<cv::Mat> *outQue)
//	{
//		m_inQue = inQue;
//		m_outQue = outQue;
//	};
//	void run() override;
//
//private:
//	QQueue<cv::Mat> *m_inQue, *m_outQue;
//	std::unique_ptr<tensorflow::Session> m_session;
//
//	std::string m_inputLayer;
//	std::vector<std::string> m_outputLayer;
//	std::vector<tensorflow::Tensor> m_outputs;
//
//	tensorflow::Tensor m_tensor;
//	tensorflow::TensorShape m_shape;
//
//	std::map<int, std::string> m_labelsMap;
//
//	static int m_count;
//};
//
