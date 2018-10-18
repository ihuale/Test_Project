#include "TFWorker.h"
#include <QDir>
#include <QString>
#include <QFileInfo>
//#include "utils.h"
#include "TileMap.h"

int TFWorker::m_count = 0;

TFWorker::TFWorker(QObject *pParent)
	:QThread(pParent),
	m_flag_model(false),
	m_flag_session(false),
	m_flag_queue(false),
	m_flag_mutex(false),
	m_flag_work_state(true)
{
	m_count++;

	//mutex
	m_mutex_queue_in = new QMutex(QMutex::RecursionMode::Recursive);
	m_mutex_queue_out = new QMutex((QMutex::RecursionMode::Recursive));

	m_sessionoptions = new tensorflow::SessionOptions;
	//m_sessionoptions->config.set_log_device_placement(true);
	m_sessionoptions->config.mutable_gpu_options()->set_allow_growth(true);
	m_sessionoptions->config.mutable_gpu_options()->set_force_gpu_compatible(true);
	m_sessionoptions->config.mutable_device_count()->insert({ "GPU",1 });

}


TFWorker::~TFWorker()
{
	if (m_count > 0)
		--m_count;

	//prevent accidental exit
	if (m_mutex_queue_in)
		m_mutex_queue_in->unlock();
	if (m_mutex_queue_out)
		m_mutex_queue_out->unlock();
	setWorkState(false);
	this->wait();
	this->quit();
}

bool TFWorker::isModelOK(std::string arg_file)
{
	if (!m_mutex_queue_in) {
		m_mutex_queue_in = new QMutex(QMutex::RecursionMode::Recursive);
	}
	m_mutex_queue_in->lock();
	//check pb is redundancy,just in case
	if (arg_file.size() < 3) {
		//just in case: D: or D:/
		m_flag_model = false;
	}
	else if (strEndWith(arg_file, ".pb") && checkFileExits(arg_file)) {
		m_flag_model = true;
	}
	//TODO
	//here lock? return in following?
	m_mutex_queue_in->unlock();

	return m_flag_model;
}

bool TFWorker::isSessionOK() const
{
	return m_flag_session;
}

Status TFWorker::LoadGraph(const std::string & graph_file_name,
	std::unique_ptr<tensorflow::Session>* session)
{
	tensorflow::GraphDef graph_def;
	Status load_graph_status =
		ReadBinaryProto(tensorflow::Env::Default(),
			graph_file_name,
			&graph_def);
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

Status TFWorker::ReadEntireFile( tensorflow::Env * env,  const std::string & filename,
	Tensor * output)
{
	tensorflow::uint64 file_size = 0;

	TF_RETURN_IF_ERROR(env->GetFileSize(filename, &file_size));

	std::string contents;
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
	output->scalar<std::string>()() = data.ToString();
	return Status::OK();
}

Status TFWorker::readTensorFromMat(
	const cv::Mat & mat, 
	Tensor & outTensor)
{
	auto root = tensorflow::Scope::NewRootScope();
	using namespace ::tensorflow::ops;

	// Trick from https://github.com/tensorflow/tensorflow/issues/8033
	float *p = outTensor.flat<float>().data();
	cv::Mat fakeMat(mat.rows, mat.cols, CV_32FC3, p);
	mat.convertTo(fakeMat, CV_32FC3);

	auto input_tensor = Placeholder(root.WithOpName("input"), 
		tensorflow::DT_FLOAT);
	std::vector<std::pair<std::string, tensorflow::Tensor>> inputs = { { "input", outTensor } };
	//for processor
	//auto uint8Caster = Cast(root.WithOpName("uint8_Cast"), outTensor, tensorflow::DT_UINT8);
	//for processor_2
	auto uint8Caster = Cast(root.WithOpName("float_Cast"), outTensor, tensorflow::DT_FLOAT);
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

Status TFWorker::readTensorMulFromMat(const std::vector<ImageMeta>* arg_list_mat,
	Tensor & outTensor)
{
	if (arg_list_mat->size() < 1)
		return Status(tensorflow::error::Code::INVALID_ARGUMENT,
			"arg_list_mat.size() < 1");

	time_t time_start, time_end;
	time_start = clock();
	int tem_size = arg_list_mat->size();
	tensorflow::Tensor tem_tensor_res(tensorflow::DataType::DT_FLOAT,
		tensorflow::TensorShape(
	{ 
		tem_size,
		m_image_width, 
		m_image_height, 
		m_image_channels 
	}));
	auto res_tensor_mapped = tem_tensor_res.tensor<float, 4>();

	unsigned int tem_batch_index = 0;//current image index
	//auto tem_mat = arg.begin();
	for (auto &tem_mat : (*arg_list_mat)) {
		if (tem_batch_index >= tem_size)
			break;

		for (unsigned int index_height = 0; index_height < m_image_height; ++index_height) {
			for (unsigned int index_width = 0; index_width < m_image_width; ++index_width) {
				for (unsigned int index_channels = 0; index_channels < m_image_channels; ++index_channels) {
					res_tensor_mapped(tem_batch_index, index_height, index_width, index_channels) =
						tem_mat.first.at<cv::Vec3b>(index_height, index_width)[index_channels];
				}
			}
		}
		++tem_batch_index;
	}
	outTensor.CopyFrom(tem_tensor_res,
		tensorflow::TensorShape({ tem_size, m_image_width, m_image_height, m_image_channels }));

	time_end = clock();
	std::cout << "[TFWorker] readTensorMulFromMat(list): "
		<< difftime(time_end, time_start) << "ms" << std::endl;

	return Status::OK();
}

Status TFWorker::ReadTensorFromImageFile(
	const std::string & file_name,
	const int input_height, 
	const int input_width,
	const float input_mean,
	const float input_std, 
	std::vector<Tensor>* out_tensors)
{
	auto root = tensorflow::Scope::NewRootScope();
	using namespace ::tensorflow::ops;  // NOLINT(build/namespaces)

	std::string input_name = "file_reader";
	std::string output_name = "normalized";

	// read file_name into a tensor named input
	Tensor input(tensorflow::DT_STRING, tensorflow::TensorShape());

	TF_RETURN_IF_ERROR(ReadEntireFile(tensorflow::Env::Default(), file_name, &input));

	// use a placeholder to read input data
	auto file_reader =
		Placeholder(root.WithOpName("input"), tensorflow::DataType::DT_STRING);

	std::vector<std::pair<std::string, tensorflow::Tensor>> inputs = {
		{ "input", input },
	};

	// Now try to figure out what kind of file it is and decode it.
	const int wanted_channels = 3;
	tensorflow::Output image_reader;
	if (strEndWith(file_name, ".png")) {
		image_reader = DecodePng(root.WithOpName("png_reader"), file_reader,
			DecodePng::Channels(wanted_channels));
	}
	else if (strEndWith(file_name, ".gif")) {
		// gif decoder returns 4-D tensor, remove the first dim
		image_reader =
			Squeeze(root.WithOpName("squeeze_first_dim"),
				DecodeGif(root.WithOpName("gif_reader"), file_reader));
	}
	else if (strEndWith(file_name, ".bmp")) {
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

void TFWorker::creatSession()
{
	if (!isModelOK(GRAPH)) {
 		LOG(ERROR) << "[TFWorker] Load graph file failed!";
		return;
	}
	if (m_flag_session) {
		LOG(INFO) << "[TFWorker] Session has been created!";
		return;
	}
	std::string tem_path_current = QDir::currentPath().toStdString();
	std::string graphPath = tensorflow::io::JoinPath(tem_path_current, GRAPH);
	LOG(INFO) << "graphPath:" << graphPath;

	m_session.reset(tensorflow::NewSession(*m_sessionoptions));
	if (!m_session) {
		LOG(ERROR) << "[TFWorker] Creating Session failed...\n";
		return;
	}
	tensorflow::Status loadGraphStatus = LoadGraph(graphPath, &m_session);
	if (!loadGraphStatus.ok()) {
		LOG(ERROR) << "[TFWorker] loadGraph(): ERROR" << loadGraphStatus;
	}
	else
		LOG(INFO) << "[TFWorker] loadGraph: frozen graph loaded" << std::endl;

	m_flag_session = true;
	m_flag_work_state = true;

	std::cout << "[Thread] num: " << m_count << std::endl;
}

bool TFWorker::setGraphFile(
	std::string arg_file, 
	std::string arg_ops_input, 
	std::vector<std::string> arg_ops_output, 
	int arg_batch_size, 
	int arg_width, 
	int arg_height, 
	int arg_channels)
{
	if (!strEndWith(arg_file, ".pb")) {
		LOG(ERROR) << "[TFWorker] arg_file is not a .pb file!" 
			<< std::endl;
		return false;
	}
	else if (!checkFileExits(arg_file)) {
		if (!m_mutex_queue_in) {
			m_mutex_queue_in = new QMutex(QMutex::RecursionMode::Recursive);
		}
		m_mutex_queue_in->lock();
		m_flag_model = false;
		m_flag_session = false;
		m_flag_work_state = false;
		m_mutex_queue_in->unlock();
		LOG(ERROR) << "[TFWorker] the file is not exits!@" << arg_file;
		return false;
	}
	if (arg_ops_input.empty() || arg_ops_output.empty()) {
		LOG(ERROR) << "[TFWorker] ops name is empty!";
		return false;
	}
	if (!(arg_batch_size&&arg_width&&arg_height&&arg_channels)) {
		LOG(ERROR) << "[TFWorker] input shape is error!";
		return false;
	}
	if (!m_mutex_queue_in) {
		m_mutex_queue_in = new QMutex(QMutex::RecursionMode::Recursive);
	}
	m_mutex_queue_in->lock();

	m_flag_model = false;
	m_flag_session = (false);
	m_flag_work_state = true;
	GRAPH = arg_file;
	m_ops_input = arg_ops_input;
	m_ops_output = arg_ops_output;

	m_batch_size = arg_batch_size;
	m_image_width = arg_width;
	m_image_height = arg_height;
	m_image_channels = arg_channels;


	//check model exits
	if (isModelOK(GRAPH)) {
		if (!m_sessionoptions) {
			m_sessionoptions = new tensorflow::SessionOptions;
		}
		//m_sessionoptions->config.set_log_device_placement(true);
		m_sessionoptions->config.mutable_gpu_options()->set_allow_growth(true);
		m_sessionoptions->config.mutable_gpu_options()->set_force_gpu_compatible(true);
		m_sessionoptions->config.mutable_device_count()->insert({ "GPU",1 });


		m_shape_input = tensorflow::TensorShape();
		m_shape_input.AddDim(m_batch_size);
		m_shape_input.AddDim((int64)m_image_width);
		m_shape_input.AddDim((int64)m_image_height);
		m_shape_input.AddDim(m_image_channels);
	}

	m_mutex_queue_in->unlock();
	
	return true;
}

void TFWorker::setOpsName(
	std::string arg_input, 
	std::vector<std::string> arg_output)
{
	if (!m_mutex_queue_in) {
		m_mutex_queue_in = new QMutex(QMutex::RecursionMode::Recursive);
	}
	m_mutex_queue_in->lock();

	if (!arg_input.empty())
		m_ops_input = arg_input;
	if (!arg_output.empty())
		m_ops_output = arg_output;

	m_mutex_queue_in->unlock();
}

void TFWorker::setShape(
	int arg_batch_size, 
	int arg_width, 
	int arg_height, 
	int arg_channels)
{
	if (!m_mutex_queue_in) {
		m_mutex_queue_in = new QMutex(QMutex::RecursionMode::Recursive);
	}
	m_mutex_queue_in->lock();

	if (arg_batch_size) {
		m_batch_size = arg_batch_size;
	}
	if (arg_width) {
		m_image_width = arg_width;
	}
	if (arg_height) {
		m_image_height = arg_height;
	}
	if (arg_channels) {
		m_image_channels = arg_channels;
	}
	m_mutex_queue_in->unlock();
}

void TFWorker::setQueues(QQueue<ImageMeta>* inQue, ResultMap* outQue)
{
	if (!m_mutex_queue_in) {
		m_mutex_queue_in = new QMutex(QMutex::RecursionMode::Recursive);
	}
	m_mutex_queue_in->lock();

	m_inQue = new QQueue<ImageMeta>;
	m_inQue = inQue;
	m_list_out = outQue;
	m_flag_queue = true;

	m_mutex_queue_in->unlock();
}

void TFWorker::setQueueMutex(QMutex& arg_in, QMutex& arg_out)
{
	//if start,do not modified the mutex
	if (isRunning()) {
		std::cout << "[TFWorker] thread is running, do not modified the mutex!\n";
		return;
	}
	m_mutex_queue_in = new QMutex(QMutex::RecursionMode::Recursive);
	m_mutex_queue_out = new QMutex(QMutex::RecursionMode::Recursive);
	m_mutex_queue_in = &arg_in;
	m_mutex_queue_out = &arg_out;
	m_flag_mutex = true;
}

void TFWorker::setWorkState(bool arg)
{
	if (!m_mutex_queue_in) {
		m_mutex_queue_in = new QMutex(QMutex::RecursionMode::Recursive);
	}
	m_mutex_queue_in->lock();

	m_flag_work_state = arg;

	m_mutex_queue_in->unlock();
}

void TFWorker::setTileMap(TileMap* arg)
{
	if (!m_mutex_queue_in) {
		m_mutex_queue_in = new QMutex(QMutex::RecursionMode::Recursive);
	}
	m_mutex_queue_in->lock();

	if (!arg) {
		return;
	}
	m_tile_map = new TileMap;
	m_tile_map = arg;
	QObject::connect(this, &TFWorker::signal_result,
		m_tile_map, &TileMap::on_signal_result,
		Qt::QueuedConnection);
	/*QObject::connect(this, SIGNAL(signal_result(ResultMeta)),
		m_tile_map, SLOT(on_signal_result(ResultMeta)),
		Qt::QueuedConnection);*/

	m_mutex_queue_in->unlock();
}

tensorflow::SessionOptions * TFWorker::mutable_session_options()
{
	if (!m_mutex_queue_in) {
		m_mutex_queue_in = new QMutex(QMutex::RecursionMode::Recursive);
	}
	m_mutex_queue_in->lock();

	if (!m_sessionoptions)
		m_sessionoptions = new tensorflow::SessionOptions();
	return m_sessionoptions;

	m_mutex_queue_in->unlock();
}

void TFWorker::run()
{
	if (!m_flag_mutex || !m_flag_queue) {
		LOG(ERROR) << "[TFWorker] mutex and queue has not assignment!!!";
		return;
	}
	if (!m_flag_session) {
		creatSession();
	}

	std::cout << "[Thread] waiting for processor worker: " 
		<< QThread::currentThreadId() 
		<< std::endl;

	std::vector<ImageMeta> list_frame_mat;

	while (m_flag_session && m_flag_work_state) {

		m_mutex_queue_in->lock();
		if (m_inQue->size() < (m_batch_size + 1)) {
			m_mutex_queue_in->unlock();

		}
		else {
			if (list_frame_mat.size() > 0) {
				list_frame_mat.clear();
			}
			for (int i = 0; i < m_batch_size; ++i) {
				auto tem_frame = m_inQue->dequeue();
				list_frame_mat.push_back(tem_frame);
			}
			std::cout << "[Thread] DeQueue one batch, size is :" 
				<< list_frame_mat.size() << std::endl;
			std::cout << "[Thread] Queue size is:" << m_inQue->size() 
				<<"-----------------------------"
				<< std::endl;
			m_mutex_queue_in->unlock();

			m_shape_input = tensorflow::TensorShape();
			m_shape_input.AddDim(m_batch_size);
			m_shape_input.AddDim((int64)m_image_width);
			m_shape_input.AddDim((int64)m_image_height);
			m_shape_input.AddDim(m_image_channels);
			m_tensor_input = Tensor(tensorflow::DT_FLOAT, m_shape_input);
			//Status readTensorStatus = readTensorFromMat(frame.first, m_tensor);///////////////////
			Status readTensorStatus = readTensorMulFromMat(&list_frame_mat, m_tensor_input);

			if (!readTensorStatus.ok()) {
				LOG(ERROR) << "[TFWorker] Mat->Tensor conversion failed: " << readTensorStatus;
			}
			m_result_outputs.clear();

			Status runStatus = m_session->Run({ { m_ops_input,m_tensor_input } },
				m_ops_output,
				{},
				&m_result_outputs);
			if (!runStatus.ok()) {
				LOG(ERROR) << "[TFWorker] Running model failed: " << runStatus;
				break;
				//return -1;
			}
			//LOG(ERROR) << m_outputs[0].shape();//for test
			for (std::size_t i = 0; i < m_result_outputs.size(); i++) {
				auto res_tensor_mapped = m_result_outputs[i].tensor<float, 2>();
#ifdef _PRINT_LOG_
				std::cout << res_tensor_mapped(0) 
					<< "	" 
					<< m_result_outputs[i].DebugString() 
					<< std::endl;
#endif //_PRINT_LOG_
				m_mutex_queue_out->lock();
				//m_list_out->enqueue(std::make_pair(frame.second, std::make_pair(res_tensor_mapped(0), 3)));
				for (int tem_index = 0; tem_index < m_result_outputs[i].dim_size(0); ++tem_index) {
					time_t tem_time = clock();
					/*m_list_out->push_back(std::make_pair(list_frame_mat[i].second,
						std::make_pair((tem_time % 100) / 100., 3)));*/
					(*m_list_out)[list_frame_mat[i].second]=
						std::make_pair(res_tensor_mapped(tem_index, 0), 3);

					//to tilemap
					emit signal_result(std::make_pair(list_frame_mat[tem_index].second,
						std::make_pair((rand() % 100), 3)));
					std::cout << "[TFWorker] emit one result!" << std::endl;
					//add_result_to_map(std::make_pair(list_frame_mat[tem_index].second,
						//std::make_pair((rand() % 100), 3)));
				}
				m_mutex_queue_out->unlock();
			}

		}

	}
}

void TFWorker::add_result_to_map(ResultMeta arg)
{
	m_mutex_queue_in->lock();
	if (m_tile_map)
		m_tile_map->add_result(arg);
	m_mutex_queue_in->unlock();
}

bool TFWorker::checkFileExits(std::string arg) const
{
	QFileInfo tem_info(QString::fromStdString(arg));
	return tem_info.exists();
}
