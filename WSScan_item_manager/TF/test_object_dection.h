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
#include <QDir>

#include "head.h"
#include "TF/types.h"

extern std::string path_image_read;

using tensorflow::Tensor;
using tensorflow::Status;

void test__run_object_dection();

tensorflow::Status readTensorMulFromMat(
	const std::vector<ImageCvMeta>* arg_list_mat,
	Tensor &outTensor);//override