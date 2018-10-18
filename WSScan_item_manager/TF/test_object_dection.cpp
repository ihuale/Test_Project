#include "test_object_dection.h"

void test__run_object_dection()
{
	tensorflow::SessionOptions *sessionoptions = new tensorflow::SessionOptions;
	sessionoptions->config.mutable_gpu_options()->set_allow_growth(true);
	sessionoptions->config.mutable_gpu_options()->set_force_gpu_compatible(true);
	sessionoptions->config.mutable_device_count()->insert({ "GPU",1 });

	std::string GRAPH = "./model/faster_rcnn/vgg16_freeze.pb";
	std::string tem_path_current = QDir::currentPath().toStdString();
	std::string graphPath = tensorflow::io::JoinPath(tem_path_current, GRAPH);
	LOG(INFO) << "graphPath:" << graphPath;

	tensorflow::Session *session = tensorflow::NewSession(*sessionoptions);

	if (!session) {
		LOG(ERROR) << "[TFWorker] Creating Session failed...\n";
		return;
	}

	tensorflow::GraphDef graph_def;
	tensorflow::Status load_graph_status =
		ReadBinaryProto(tensorflow::Env::Default(),
			graphPath,
			&graph_def);
	if (!load_graph_status.ok()) {
		LOG(ERROR) << "Failed to load compute graph at '" + graphPath + "'";
		return;
	}
	tensorflow::Status session_create_status = session->Create(graph_def);
	if (!session_create_status.ok()) {
		LOG(ERROR) << "Failed to load compute graph at '" << session_create_status;
		return;
	}

	//creat test data
	std::vector<ImageCvMeta> list_frame_mat;

	auto shape_input = tensorflow::TensorShape();
	shape_input.AddDim(4);//m_batch_size
	shape_input.AddDim((int64)(299));//m_image_width
	shape_input.AddDim((int64)(299));//m_image_height
	shape_input.AddDim(3);//m_image_channels
	auto tensor_input = tensorflow::Tensor(tensorflow::DT_FLOAT, shape_input);
	//Status readTensorStatus = readTensorFromMat(frame.first, m_tensor);///////////////////
	tensorflow::Status readTensorStatus = readTensorMulFromMat(&list_frame_mat, tensor_input);

	if (!readTensorStatus.ok()) {
		LOG(ERROR) << "[Test] Mat->Tensor conversion failed: " << readTensorStatus;
	}
	std::vector<Tensor> result_outputs;

	std::string ops_input = "";
	std::vector<std::string> ops_output = { "" };

	Status runStatus = session->Run({ { ops_input,tensor_input } },
		ops_output,
		{},
		&result_outputs);
	if (!runStatus.ok()) {
		LOG(ERROR) << "[TFWorker] Running model failed: " << runStatus;
		return;
	}

}

tensorflow::Status readTensorMulFromMat(
	const std::vector<ImageCvMeta>* arg_list_mat,
	Tensor &outTensor)
{
	if (arg_list_mat->size() < 1)
		return Status(tensorflow::error::Code::INVALID_ARGUMENT,
			"arg_list_mat.size() < 1");

	int tem_width = 299, tem_height = 299, tem_channels = 3;
	time_t time_start, time_end;
	time_start = clock();
	int tem_size = arg_list_mat->size();
	tensorflow::Tensor tem_tensor_res(tensorflow::DataType::DT_FLOAT,
		tensorflow::TensorShape(
	{
		tem_size,
		tem_width,
		tem_height,
		tem_channels
	}));
	auto res_tensor_mapped = tem_tensor_res.tensor<float, 4>();

	unsigned int tem_batch_index = 0;//current image index
									 //auto tem_mat = arg.begin();
	for (auto &tem_mat : (*arg_list_mat)) {
		if (tem_batch_index >= tem_size)
			break;

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
		tensorflow::TensorShape({ tem_size, tem_width, tem_height, tem_channels }));

	time_end = clock();
	std::cout << "[TFWorker] readTensorMulFromMat(list): "
		<< difftime(time_end, time_start) << "ms" << std::endl;

	return Status::OK();
}
