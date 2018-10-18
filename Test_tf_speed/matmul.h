#pragma once
#ifndef MATMUL_H
#define MATMUL_H

#ifndef COMPILER_MSVC
#define COMPILER_MSVC
#endif // !COMPILER_MSVC

#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX

#include <fstream>
#include <utility>
#include <vector>
#include <thread>
#include <ctime>
#include <algorithm>
#include <random>

#include <Eigen/Core>
#include <Eigen/Dense>

//#include "tensorflow/cc/ops/const_op.h"
//#include "tensorflow/cc/ops/image_ops.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/protobuf/config.pb.h"
//#include "tensorflow/core/protobuf/config.pb.cc"
#include "tensorflow/core/protobuf/meta_graph.pb.h"
//#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/graph/default_device.h"
//#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/common_runtime/device_factory.h"
#include "tensorflow/core/lib/core/errors.h"
//#include "tensorflow/core/lib/core/threadpool.h"
//#include "tensorflow/core/lib/io/path.h"
//#include "tensorflow/core/lib/core/stringpiece.h"
//#include "google/protobuf/stubs/stringpiece.h"
#include "tensorflow/core/lib/strings/numbers.h"
#include "tensorflow/core/lib/strings/str_util.h"
//#include "tensorflow/core/lib/strings/stringprintf.h"
//#include "tensorflow/core/platform/env.h"
//#include "tensorflow/core/platform/init_main.h"
//#include "tensorflow/core/platform/logging.h"
//#include "tensorflow/core/platform/types.h"
//#include "tensorflow/core/util/command_line_flags.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/util/port.h"
#include "tensorflow/core/util/device_name_utils.h"
//#include "tensorflow/core/distributed_runtime/rpc/grpc_session.h"

//using namespace std;
//using namespace tensorflow;
//using namespace tensorflow::ops;
//using namespace tensorflow::strings;
//using tensorflow::Flag;
//using tensorflow::Tensor;
//using tensorflow::Status;
//using tensorflow::string;
//using tensorflow::int32;
//using tensorflow::int64;

#endif //MATMUL_H