#pragma once

#ifndef COMPILER_MSVC
#define COMPILER_MSVC
#endif //COMPILER_MSVC

#ifndef NOMINMAX
#define NOMINMAX
#endif //NOMINMAX

#include <stdio.h>
#include <tchar.h>

#include <numeric>
#include <eigen/Dense>
#include <fstream>
#include <utility>
#include <vector>
#include <list>
#include <fstream>
#include <iostream>
#include <regex>

#include "tensorflow/cc/ops/const_op.h"
#include "tensorflow/cc/ops/image_ops.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/lib/strings/str_util.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/util/command_line_flags.h"

#include <opencv2/opencv.hpp>

#include <QQueue>
#include <QMutex>
#include <QVector>
#include <QThread>
#include <QRunnable>
#include <QThreadPool>
#include <QObject>


typedef unsigned int ImageID;
typedef std::pair<cv::Mat, ImageID> ImageMeta;

//ResultMeta: <<iamge, id>,<score, class_id>>
//ScoreMeta: <score, class_id>
typedef std::pair<double, int> ScoreMeta;
typedef std::pair<ImageID, ScoreMeta> ResultMeta;
//typedef std::pair<ImageID, ScoreMeta> ResultMeta;
typedef std::map<ImageID, ScoreMeta> ResultMap;

// These are all common classes it's handy to reference with no namespace.
using tensorflow::Flag;
using tensorflow::Tensor;
using tensorflow::Status;
using tensorflow::string;
using tensorflow::int32;

#ifndef __TOOL_FUNCTION__
#define __MY_UTILS__
#define __TOOL_FUNCTION__

bool StrEndWith(const std::string &str, const std::string &tail);

bool compare(ResultMeta arg_a, ResultMeta arg_b);

#endif __TOOL_FUNCTION__

Status LoadGraph(const string& graph_file_name,
		std::unique_ptr<tensorflow::Session>* session);

Status readLabelsMapFile(const string &fileName, std::map<int, string> &labelsMap);


// Given an image file name, read in the data, try to decode it as an image,
// resize it to the requested size, and then scale the values as desired.
Status ReadTensorFromImageFile(
	const string& file_name, 
	const int input_height,
	const int input_width,
	const float input_mean,
	const float input_std,
	std::vector<Tensor>* out_tensors);
Status readTensorFromMat(const cv::Mat &mat,
	Tensor &outTensor);
Status readTensorMulFromMat(const std::vector<ImageMeta>* arg_list_mat, 
	Tensor &outTensor);//override


std::vector<size_t> filterBoxes(tensorflow::TTypes<float>::Flat &scores,
		tensorflow::TTypes<float, 3>::Tensor &boxes,
		double thresholdIOU, double thresholdScore);

void drawBoundingBoxesOnImage(cv::Mat &image,
		tensorflow::TTypes<float>::Flat &scores,
		tensorflow::TTypes<float>::Flat &classes,
		tensorflow::TTypes<float, 3>::Tensor &boxes,
		std::map<int, string> &labelsMap,
		std::vector<size_t> &idxs);

