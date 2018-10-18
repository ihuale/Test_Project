#pragma once
#include "matmul.h"
#include <mutex>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>

//Note:
//Traditional pointers are prone to errors
//and later consider using smart pointers.

//image coordinate: (image_coor_x, image_coor_y)
typedef std::pair<unsigned int, unsigned int> ImageCoordinate;

//cv::Mat pair: (image_data, (image_coor_x, image_coor_y))
typedef std::pair<cv::Mat*, ImageCoordinate > MatPair;
typedef std::list<MatPair*> MatList;

//Tensor(batch_size, image.height, image.width, image.channels)
typedef std::pair<tensorflow::Tensor, std::list<ImageCoordinate> > TensorList;

//result pair: (score, (image_coor_x, image_coor_y))
typedef std::pair<unsigned int, ImageCoordinate> ResultPair;
typedef std::list<ResultPair> ResultList;

class TFManager
{
public:
	TFManager();
	~TFManager();

	//job
	bool getImageBatch(MatList& res, unsigned int batch_size);
	bool addImage(MatPair* arg);
	bool addImageBatch(MatList& arg);

	//tf config
	tensorflow::SessionOptions* mutable_session_options();
	bool initializeTF();

	//tf run prediction
	void setWorkState(bool arg);
	void prediction();

	//tf result
	void getAllResult(ResultList& res);

private:
	//pimpl for tf
	class TFWorker;
	TFWorker* tfworker_;

	//tf config
	tensorflow::SessionOptions* options_;

	//job
	bool abort_;//for multi thread
	unsigned int thread_waiting_;//for multi thread

	TF_DISALLOW_COPY_AND_ASSIGN(TFManager);
};


class TFManager::TFWorker
{
public:
	TFWorker(unsigned int thread_num = 1);
	~TFWorker();

	//initialize
	tensorflow::Status initializeSession(
		tensorflow::SessionOptions &arg_options = tensorflow::SessionOptions());

	//prediction
	void prediction();

	//for state
	bool isGraphdefInitialize();
	bool isSessionInitialize();

	//job
	bool getImageBatch(MatList& res, unsigned int batch);
	bool addImage(MatPair* arg);
	bool addImageBatch(MatList& arg);
	void setWorkState(bool arg);
	void setBatchSize(unsigned int arg);

	//result
	void getAllResult(ResultList& res);
	void addResult(ResultPair& arg);
	void addResultBatch(ResultList& arg);

	//data translate
	//translate arg to res
	static bool cvMatToTensor(MatList& arg,
		TensorList& res);

private:
	//thread
	std::vector<std::thread*> th_list_predict_;

	//tf
	//tensorflow::Scope* scope_;//unused!
	tensorflow::Session* session_;
	tensorflow::GraphDef graphdef_;
	unsigned int batch_size_;
	//ops name
	std::string ops_output_;
	std::string ops_input_;

	//predict result
	ResultList result_list_;
	std::unique_lock<std::mutex> result_list_mutex_;//for multi thread

	//path of file, relative path
	std::string path_pb_;

	//flag for initialize
	bool flag_graph_;
	bool flag_session_;
	bool flag_work_state_;

	//job
	bool abort_;//for multi thread
	unsigned int thread_waiting_;//for multi thread
	std::unique_lock<std::mutex> job_list_mutex_;//for multi thread
	MatList job_list_;

	TF_DISALLOW_COPY_AND_ASSIGN(TFWorker);
};