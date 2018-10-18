#pragma once
#ifndef COMPILER_MSVC
#define COMPILER_MSVC
#endif //COMPILER_MSVC

#ifndef NOMINMAX
#define NOMINMAX
#endif //NOMINMAX

#include <numeric>
#include <fstream>
#include <utility>
#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <iostream>
#include <regex>

#include <eigen/Dense>
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/public/session_options.h"

#include <opencv2/opencv.hpp>

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QMutex>

class TileMap;

typedef unsigned int ImageID;
typedef std::pair<cv::Mat, ImageID> ImageMeta;

//ResultMeta: <<iamge, id>,<score, class_id>>
//ScoreMeta: <score, class_id>
typedef std::pair<double, int> ScoreMeta;
typedef std::pair<ImageID, ScoreMeta> ResultMeta;
//typedef std::pair<ImageID, ScoreMeta> ResultMeta;
typedef std::map<ImageID, ScoreMeta> ResultMap;

using tensorflow::Status;
using tensorflow::Tensor;

class TFWorker :
	public QThread
{
	Q_OBJECT
public:
	explicit TFWorker(QObject *pParent = nullptr);
	virtual ~TFWorker();

	bool isModelOK(std::string arg_file);
	bool isSessionOK() const;


	/********************for tensorlow config************************/
	//for tensorlfow
	Status LoadGraph(const std::string& graph_file_name,
		std::unique_ptr<tensorflow::Session>* session);

	Status ReadEntireFile(
		tensorflow::Env* env,
		const std::string& filename,
		Tensor* output);

	Status readTensorFromMat(
		const cv::Mat &mat,
		Tensor &outTensor);

	Status readTensorMulFromMat(
		const std::vector<ImageMeta>* arg_list_mat,
		Tensor &outTensor);//override

	// given an image file name, read in the data, 
	// try to decode it as an image,
	// resize it to the requested size, 
	// and then scale the values as desired.
	Status ReadTensorFromImageFile(
		const std::string& file_name,
		const int input_height,
		const int input_width, 
		const float input_mean,
		const float input_std,
		std::vector<Tensor>* out_tensors);

	//for config the tensorlow
	tensorflow::SessionOptions* mutable_session_options();
	/********************for tensorlow config************************/


	/********************for TFWorker config************************/

	//if all is ok,then we creat session,
	//only creat session is ok,
	//the thread will run
	void creatSession();

	//only all arg is ok,
	//TFWorker can work!!!
	bool setGraphFile(
		std::string arg_file,
		std::string arg_ops_input,
		std::vector<std::string> arg_ops_output,
		int arg_batch_size,
		int arg_width,
		int arg_height,
		int arg_channels);

	//for ops config
	void setOpsName(std::string arg_input = "input_1:0",
		std::vector<std::string> arg_output = 
	{ "dense_2/Sigmoid:0"});

	//for input ops config
	void setShape(
		int arg_batch_size = 4,
		int arg_width = 512,
		int arg_height = 512,
		int arg_channels = 3);

	void setQueues(QQueue<ImageMeta> *inQue,
		ResultMap *outQue);

	void setQueueMutex(QMutex& arg_in, QMutex& arg_out);

	void setWorkState(bool arg);

	//for tilemap test
	void setTileMap(TileMap* arg);

	/********************for TFWorker config************************/

	//thread
	void run() override;

signals:
	//dynamic display for TileMap
	void signal_result(ResultMeta arg);

public slots:
	void add_result_to_map(ResultMeta arg);


private:
	//disable copy
	/*TFWorker();*/
	TFWorker(TFWorker*);
	TFWorker(TFWorker&);

	//for test tilemap
	TileMap* m_tile_map;


	QQueue<ImageMeta> *m_inQue;
	//std::list<ResultMeta> *m_list_out;
	ResultMap *m_list_out;
	QMutex *m_mutex_queue_in, *m_mutex_queue_out;
	std::unique_ptr<tensorflow::Session> m_session;

	//tf graph congif
	std::string LABELS;//not used
	std::string GRAPH;
	std::string m_ops_input;
	std::vector<std::string> m_ops_output;
	std::vector<tensorflow::Tensor> m_result_outputs;

	tensorflow::Tensor m_tensor_input;
	tensorflow::TensorShape m_shape_input;
	tensorflow::SessionOptions* m_sessionoptions;

	//for image config
	int m_batch_size;
	int m_image_width;
	int m_image_height;
	int m_image_channels;

	//map of result:<id, name>
	//std::map<int, std::string> m_labelsMap;

	//for thread count
	static int m_count;

	//flag
	//true: found *.pb file
	bool m_flag_model;
	//true; initialize of session has over
	bool m_flag_session;
	//true: has extern queue, false: no extern queue
	bool m_flag_queue;
	//true: has extern mutex, false: no extern mutex
	bool m_flag_mutex;
	//treu::run, false:stop
	bool m_flag_work_state;

	//tool function
	bool checkFileExits(std::string arg) const;
	bool strEndWith(const std::string &str, const std::string &tail)
	{
		return str.compare(str.size() - tail.size(), tail.size(), tail) == 0;
	}//StrEndWit

	bool compare(ResultMeta arg_a, ResultMeta arg_b)
	{
		//descending sort
		return arg_a.second.first > arg_b.second.first;

		//ascending sort
		//return arg_a.second.first > arg_b.second.first;
	}//compare
};