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
#include <fstream>
#include <iostream>
#include <regex>
#include <ctime>

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
#include "utils.h"

//#include "CheckOutputQue.h"


// These are all common classes it's handy to reference with no namespace.
using tensorflow::Flag;
using tensorflow::Tensor;
using tensorflow::Status;
using tensorflow::string;
using tensorflow::int32;

#ifdef __MY_UTILS__
#ifdef __TOOL_FUNCTION__

bool StrEndWith(const std::string &str, const std::string &tail) {
		return str.compare(str.size() - tail.size(), tail.size(), tail) == 0;
	}//StrEndWit

bool compare(ResultMeta arg_a, ResultMeta arg_b)
{
	//descending sort
	return arg_a.second.first > arg_b.second.first;

	//ascending sort
	//return arg_a.second.first > arg_b.second.first;
}

#endif //__TOOL_FUNCTION__
#endif //__MY_UTILS__

// Takes a file name, and loads a list of labels from it, one per line, and
// returns a vector of the strings. It pads with empty strings so the length
// of the result is a multiple of 16, because our model expects that.
Status ReadLabelsFile(const string& file_name, std::vector<string>* result,
	size_t* found_label_count) {
	std::ifstream file(file_name);
	if (!file) {
		return tensorflow::errors::NotFound("Labels file ", file_name, " not found.");
	}
	result->clear();
	string line;
	while (std::getline(file, line)) {
		result->push_back(line);
	}
	*found_label_count = result->size();
	const int padding = 16;
	while (result->size() % padding) {
		result->emplace_back();
	}
	return Status::OK();
}

Status ReadEntireFile(
	tensorflow::Env* env, 
	const string& filename,
	Tensor* output) 
{
	tensorflow::uint64 file_size = 0;

	TF_RETURN_IF_ERROR(env->GetFileSize(filename, &file_size));

	string contents;
	contents.resize(file_size);

	std::unique_ptr<tensorflow::RandomAccessFile> file;
	TF_RETURN_IF_ERROR(env->NewRandomAccessFile(filename, &file));

	tensorflow::StringPiece data;
	TF_RETURN_IF_ERROR(file->Read(0, file_size, &data, &(contents)[0]));
	if (data.size() != file_size) {
		return tensorflow::errors::DataLoss("Truncated read of '", filename,
			"' expected ", file_size, " got ",
			data.size());
	}
	output->scalar<string>()() = data.ToString();
	return Status::OK();
}

// Given an image file name, read in the data, try to decode it as an image,
// resize it to the requested size, and then scale the values as desired.
Status ReadTensorFromImageFile(
	const string& file_name,
	const int input_height,
	const int input_width, 
	const float input_mean,
	const float input_std,
	std::vector<Tensor>* out_tensors) 
{
	auto root = tensorflow::Scope::NewRootScope();
	using namespace ::tensorflow::ops;  // NOLINT(build/namespaces)

	string input_name = "file_reader";
	string output_name = "normalized";

	// read file_name into a tensor named input
	Tensor input(tensorflow::DT_STRING, tensorflow::TensorShape());

	TF_RETURN_IF_ERROR(ReadEntireFile(tensorflow::Env::Default(), file_name, &input));

	// use a placeholder to read input data
	auto file_reader =
		Placeholder(root.WithOpName("input"), tensorflow::DataType::DT_STRING);

	std::vector<std::pair<string, tensorflow::Tensor>> inputs = {
		{ "input", input },
	};

	// Now try to figure out what kind of file it is and decode it.
	const int wanted_channels = 3;
	tensorflow::Output image_reader;
	if (StrEndWith(file_name, ".png")) {
		image_reader = DecodePng(root.WithOpName("png_reader"), file_reader,
			DecodePng::Channels(wanted_channels));
	}
	else if (StrEndWith(file_name, ".gif")) {
		// gif decoder returns 4-D tensor, remove the first dim
		image_reader =
			Squeeze(root.WithOpName("squeeze_first_dim"),
				DecodeGif(root.WithOpName("gif_reader"), file_reader));
	}
	else if (StrEndWith(file_name, ".bmp")) {
		image_reader = DecodeBmp(root.WithOpName("bmp_reader"), file_reader);
	}
	else {
		// Assume if it's neither a PNG nor a GIF then it must be a JPEG.
		image_reader = DecodeJpeg(root.WithOpName("jpeg_reader"), file_reader,
			DecodeJpeg::Channels(wanted_channels));
	}
	// Now cast the image data to float so we can do normal math on it.
	auto float_caster =
		Cast(root.WithOpName("float_caster"), image_reader, tensorflow::DT_FLOAT);
	// The convention for image ops in TensorFlow is that all images are expected
	// to be in batches, so that they're four-dimensional arrays with indices of
	// [batch, height, width, channel]. Because we only have a single image, we
	// have to add a batch dimension of 1 to the start with ExpandDims().
	auto dims_expander = ExpandDims(root, float_caster, 0);
	// Bilinearly resize the image to fit the required dimensions.
	auto resized = ResizeBilinear(
		root, dims_expander,
		Const(root.WithOpName("size"), { input_height, input_width }));
	// Subtract the mean and divide by the scale.
	Div(root.WithOpName(output_name), Sub(root, resized, { input_mean }),
		{ input_std });

	// This runs the GraphDef network definition that we've just constructed, and
	// returns the results in the output tensor.
	tensorflow::GraphDef graph;
	TF_RETURN_IF_ERROR(root.ToGraphDef(&graph));

	std::unique_ptr<tensorflow::Session> session(
		tensorflow::NewSession(tensorflow::SessionOptions()));
	TF_RETURN_IF_ERROR(session->Create(graph));
	TF_RETURN_IF_ERROR(session->Run({ inputs }, { output_name }, {}, out_tensors));
	return Status::OK();
}

// Reads a model graph definition from disk, and creates a session object you
// can use to run it.
Status LoadGraph(const string& graph_file_name,
	std::unique_ptr<tensorflow::Session>* session) 
{
	tensorflow::GraphDef graph_def;
	Status load_graph_status =
		ReadBinaryProto(tensorflow::Env::Default(), graph_file_name, &graph_def);
	if (!load_graph_status.ok()) {
		return tensorflow::errors::NotFound("Failed to load compute graph at '",
			graph_file_name, "'");
	}
	session->reset(tensorflow::NewSession(tensorflow::SessionOptions()));
	Status session_create_status = (*session)->Create(graph_def);
	if (!session_create_status.ok()) {
		return session_create_status;
	}
	return Status::OK();
}

// Analyzes the output of the Inception graph to retrieve the highest scores and
// their positions in the tensor, which correspond to categories.
Status GetTopLabels(const std::vector<Tensor>& outputs, int how_many_labels,
	Tensor* indices, Tensor* scores) {
	auto root = tensorflow::Scope::NewRootScope();
	using namespace ::tensorflow::ops;  // NOLINT(build/namespaces)

	string output_name = "top_k";
	TopK(root.WithOpName(output_name), outputs[0], how_many_labels);
	// This runs the GraphDef network definition that we've just constructed, and
	// returns the results in the output tensors.
	tensorflow::GraphDef graph;

	TF_RETURN_IF_ERROR(root.ToGraphDef(&graph));

	std::unique_ptr<tensorflow::Session> session(
		tensorflow::NewSession(tensorflow::SessionOptions()));
	TF_RETURN_IF_ERROR(session->Create(graph));
	// The TopK node returns two outputs, the scores and their original indices,
	// so we have to append :0 and :1 to specify them both.
	std::vector<Tensor> out_tensors;
	TF_RETURN_IF_ERROR(session->Run({}, { output_name + ":0", output_name + ":1" },
		{}, &out_tensors));
	*scores = out_tensors[0];
	*indices = out_tensors[1];
	return Status::OK();
}

// Given the output of a model run, and the name of a file containing the labels
// this prints out the top five highest-scoring values.
Status PrintTopLabels(const std::vector<Tensor>& outputs,
	const string& labels_file_name) {
	std::vector<string> labels;
	size_t label_count;
	Status read_labels_status =
		ReadLabelsFile(labels_file_name, &labels, &label_count);
	if (!read_labels_status.ok()) {
		LOG(ERROR) << read_labels_status;
		return read_labels_status;
	}
	const int how_many_labels = std::min(5, static_cast<int>(label_count));
	Tensor indices;
	Tensor scores;
	TF_RETURN_IF_ERROR(GetTopLabels(outputs, how_many_labels, &indices, &scores));
	tensorflow::TTypes<float>::Flat scores_flat = scores.flat<float>();
	tensorflow::TTypes<int32>::Flat indices_flat = indices.flat<int32>();
	for (int pos = 0; pos < how_many_labels; ++pos) {
		const int label_index = indices_flat(pos);
		const float score = scores_flat(pos);
		LOG(INFO) << labels[label_index] << " (" << label_index << "): " << score;
	}
	return Status::OK();
}

// This is a testing function that returns whether the top label index is the
// one that's expected.
Status CheckTopLabel(const std::vector<Tensor>& outputs, int expected,
	bool* is_expected) {
	*is_expected = false;
	Tensor indices;
	Tensor scores;
	const int how_many_labels = 1;
	TF_RETURN_IF_ERROR(GetTopLabels(outputs, how_many_labels, &indices, &scores));
	tensorflow::TTypes<int32>::Flat indices_flat = indices.flat<int32>();
	if (indices_flat(0) != expected) {
		LOG(ERROR) << "Expected label #" << expected << " but got #"
			<< indices_flat(0);
		*is_expected = false;
	}
	else {
		*is_expected = true;
	}
	return Status::OK();
}

Status readLabelsMapFile(const string &fileName, std::map<int, string> &labelsMap) {

	// Read file into a string
	std::ifstream t(fileName);
	if (t.bad())
		return tensorflow::errors::NotFound("Failed to load labels map at '", fileName, "'");
	std::stringstream buffer;
	buffer << t.rdbuf();
	string fileString = buffer.str();

	// Search entry patterns of type 'item { ... }' and parse each of them
	std::smatch matcherEntry;
	std::smatch matcherId;
	std::smatch matcherName;
	const std::regex reEntry("item \\{([\\S\\s]*?)\\}");
	const std::regex reId("[0-9]+");
	const std::regex reName("\'.+\'");
	string entry;

	auto stringBegin = std::sregex_iterator(fileString.begin(), fileString.end(), reEntry);
	auto stringEnd = std::sregex_iterator();

	int id;
	string name;
	for (std::sregex_iterator i = stringBegin; i != stringEnd; i++) {
		matcherEntry = *i;
		entry = matcherEntry.str();
		regex_search(entry, matcherId, reId);
		if (!matcherId.empty())
			id = stoi(matcherId[0].str());
		else
			continue;
		regex_search(entry, matcherName, reName);
		if (!matcherName.empty())
			name = matcherName[0].str().substr(1, matcherName[0].str().length() - 2);
		else
			continue;
		labelsMap.insert(std::pair<int, string>(id, name));
	}
	return Status::OK();
}

/** Convert Mat image into tensor of shape (1, height, width, d) where last three dims are equal to the original dims.
*/
Status readTensorFromMat(const cv::Mat &mat, Tensor &outTensor) {

	auto root = tensorflow::Scope::NewRootScope();
	using namespace ::tensorflow::ops;

	// Trick from https://github.com/tensorflow/tensorflow/issues/8033
	float *p = outTensor.flat<float>().data();
	cv::Mat fakeMat(mat.rows, mat.cols, CV_32FC3, p);
	mat.convertTo(fakeMat, CV_32FC3);

	auto input_tensor = Placeholder(root.WithOpName("input"), tensorflow::DT_FLOAT);
	std::vector<std::pair<string, tensorflow::Tensor>> inputs = { { "input", outTensor } };
	//auto uint8Caster = Cast(root.WithOpName("uint8_Cast"), outTensor, tensorflow::DT_UINT8);//for processor
	auto uint8Caster = Cast(root.WithOpName("float_Cast"), outTensor, tensorflow::DT_FLOAT);//for processor_2

	// This runs the GraphDef network definition that we've just constructed, and
	// returns the results in the output outTensor.
	tensorflow::GraphDef graph;
	TF_RETURN_IF_ERROR(root.ToGraphDef(&graph));

	std::vector<Tensor> outTensors;
	std::unique_ptr<tensorflow::Session> session(tensorflow::NewSession(tensorflow::SessionOptions()));

	TF_RETURN_IF_ERROR(session->Create(graph));
	//TF_RETURN_IF_ERROR(session->Run({ inputs }, { "uint8_Cast" }, {}, &outTensors));//for processor
	TF_RETURN_IF_ERROR(session->Run({ inputs }, { "float_Cast" }, {}, &outTensors));//for processor_2

	outTensor = outTensors.at(0);
	return Status::OK();
}

Status readTensorMulFromMat(const std::vector<ImageMeta>* arg_list_mat, Tensor& outTensor)
{
	if (arg_list_mat->size() < 1)//utils_batch_size::32
		return Status( tensorflow::error::Code::INVALID_ARGUMENT,
			"arg_list_mat.size() < 1");//utils_batch_size::32
	
	time_t time_start, time_end;
 	time_start = clock();
	int tem_size = arg_list_mat->size();
	tensorflow::Tensor tem_tensor_res(tensorflow::DataType::DT_FLOAT,
		tensorflow::TensorShape({ tem_size, 512, 512, 3 }));//utils_batch_size::32
	auto res_tensor_mapped = tem_tensor_res.tensor<float, 4>();

	unsigned int tem_batch_index = 0;//current image index
	//auto tem_mat = arg.begin();
	for (auto &tem_mat : (*arg_list_mat)) {
		if (tem_batch_index >= tem_size)//utils_batch_size::32
			break;

		//auto tem_height = tem_mat.rows;
		//auto tem_width = tem_mat.cols;
		auto tem_height = 512;
		auto tem_width = 512;
		auto tem_channels = tem_mat.first.channels();

		for (unsigned int index_height = 0; index_height < tem_height; ++index_height) {
			for (unsigned int index_width = 0; index_width < tem_width; ++index_width) {
				for (unsigned int index_channels = 0; index_channels < tem_channels; ++index_channels) {
					res_tensor_mapped(tem_batch_index, index_height, index_width, index_channels) =
						tem_mat.first.at<cv::Vec3b>(index_height, index_width)[index_channels];
				}
			}
		}
		++tem_batch_index;
	}
	outTensor.CopyFrom(tem_tensor_res,
		tensorflow::TensorShape({ tem_size, 512, 512, 3 }));//utils_batch_size::32

	time_end = clock();
	std::cout << "[Utils] readTensorFromMat(list): " 
		<< difftime(time_end, time_start) << "ms" << std::endl;

	return Status::OK();
}

/** Calculate intersection-over-union (IOU) for two given bbox Rects.
*/
double IOU(cv::Rect2f box1, cv::Rect2f box2) {

	float xA = std::max(box1.tl().x, box2.tl().x);
	float yA = std::max(box1.tl().y, box2.tl().y);
	float xB = std::min(box1.br().x, box2.br().x);
	float yB = std::min(box1.br().y, box2.br().y);

	float intersectArea = abs((xB - xA) * (yB - yA));
	float unionArea = abs(box1.area()) + abs(box2.area()) - intersectArea;

	return 1. * intersectArea / unionArea;
}

/** Return idxs of good boxes (ones with highest confidence score (>= thresholdScore)
*  and IOU <= thresholdIOU with others).
*/
std::vector<size_t> filterBoxes(tensorflow::TTypes<float>::Flat &scores,
	tensorflow::TTypes<float, 3>::Tensor &boxes,
	double thresholdIOU, double thresholdScore) {

	std::vector<size_t> sortIdxs(scores.size());
	std::iota(sortIdxs.begin(), sortIdxs.end(), 0);

	// Create set of "bad" idxs
	std::set<size_t> badIdxs = std::set<size_t>();
	size_t i = 0;
	while (i < sortIdxs.size()) {
		if (scores(sortIdxs.at(i)) < thresholdScore)
			badIdxs.insert(sortIdxs[i]);
		if (badIdxs.find(sortIdxs.at(i)) != badIdxs.end()) {
			i++;
			continue;
		}

		cv::Rect2f box1 = cv::Rect2f(cv::Point2f(boxes(0, sortIdxs.at(i), 1), boxes(0, sortIdxs.at(i), 0)),
			cv::Point2f(boxes(0, sortIdxs.at(i), 3), boxes(0, sortIdxs.at(i), 2)));
		for (size_t j = i + 1; j < sortIdxs.size(); j++) {
			if (scores(sortIdxs.at(j)) < thresholdScore) {
				badIdxs.insert(sortIdxs[j]);
				continue;
			}
			cv::Rect2f box2 = cv::Rect2f(cv::Point2f(boxes(0, sortIdxs.at(j), 1), boxes(0, sortIdxs.at(j), 0)),
				cv::Point2f(boxes(0, sortIdxs.at(j), 3), boxes(0, sortIdxs.at(j), 2)));
			if (IOU(box1, box2) > thresholdIOU)
				badIdxs.insert(sortIdxs[j]);
		}
		i++;
	}

	// Prepare "good" idxs for return
	std::vector<size_t> goodIdxs = std::vector<size_t>();
	for (auto it = sortIdxs.begin(); it != sortIdxs.end(); it++)
		if (badIdxs.find(sortIdxs.at(*it)) == badIdxs.end())
			goodIdxs.push_back(*it);

	return goodIdxs;
}

void drawBoundingBoxOnImage(cv::Mat &image, double yMin, double xMin, double yMax, double xMax, double score, string label, bool scaled = true) {
	cv::Point tl, br;
	if (scaled) {
		tl = cv::Point((int)(xMin * image.cols), (int)(yMin * image.rows));
		br = cv::Point((int)(xMax * image.cols), (int)(yMax * image.rows));
	}
	else {
		tl = cv::Point((int)xMin, (int)yMin);
		br = cv::Point((int)xMax, (int)yMax);
	}
	cv::rectangle(image, tl, br, cv::Scalar(0, 255, 255), 1);

	// Ceiling the score down to 3 decimals (weird!)
	float scoreRounded = floorf(score * 1000) / 1000;
	string scoreString = std::to_string(scoreRounded).substr(0, 5);
	string caption = label + " (" + scoreString + ")";

	// Adding caption of type "LABEL (X.XXX)" to the top-left corner of the bounding box
	int fontCoeff = 12;
	cv::Point brRect = cv::Point(tl.x + caption.length() * fontCoeff / 1.6, tl.y + fontCoeff);
	cv::rectangle(image, tl, brRect, cv::Scalar(0, 255, 255), -1);
	cv::Point textCorner = cv::Point(tl.x, tl.y + fontCoeff * 0.9);
	cv::putText(image, caption, textCorner, cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 0));
}

void drawBoundingBoxesOnImage(cv::Mat &image,
	tensorflow::TTypes<float>::Flat &scores,
	tensorflow::TTypes<float>::Flat &classes,
	tensorflow::TTypes<float, 3>::Tensor &boxes,
	std::map<int, string> &labelsMap,
	std::vector<size_t> &idxs) {
	/*std::cout <<"label 0 is: "<< labelsMap[0] << std::endl;
	std::cout << "label 1 is: " << labelsMap[1] << std::endl;*/

	for (int j = 0; j < idxs.size(); j++)
		drawBoundingBoxOnImage(image,
			boxes(0, idxs.at(j), 0), boxes(0, idxs.at(j), 1),
			boxes(0, idxs.at(j), 2), boxes(0, idxs.at(j), 3),
			scores(idxs.at(j)), labelsMap[classes(idxs.at(j))]);
}