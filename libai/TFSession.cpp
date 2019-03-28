#include "TF.h"
#include "ImgPreProcess.h"

TFSession::TFSession() :
	sessionOK(false)
{
}

TFSession::~TFSession()
{
	if (session.get()) {
		session->Close();
	}
}

bool TFSession::setGraphPath(std::string arg_pbPath)
{
	sessionOK = false;
	FILE *fp;
	auto err = fopen_s(&fp, arg_pbPath.c_str(), "r");
	if (0 != err) {
		printf("[TFSession] file does not exits: %s\n", arg_pbPath.c_str());
		return false;
	}
	fclose(fp);
	tensorflow::GraphDef graph_def;
	Status load_graph_status =
		ReadBinaryProto(tensorflow::Env::Default(), arg_pbPath, &graph_def);
	if (!load_graph_status.ok()) {
		printf("[TFSession] Failed to load compute graph: %s\n", arg_pbPath.c_str());
		return false;
	}
	session.reset(tensorflow::NewSession(tensorflow::SessionOptions()));
	auto session_creat_status = session.get()->Create(graph_def);
	if (!session_creat_status.ok()) {
		printf("[TFSession] creat session failed: %s\n", arg_pbPath.c_str());
		return false;
	}

	m_pbPath = arg_pbPath;
	sessionOK = true;

	return true;
}

bool TFSession::preatingRun()
{
	if (!sessionOK) {
		printf("[TFSession] preatingRun failed, because session is not ok!\n");
		return false;
	}

	ImgCVMeta temMeta;
	int tem_batch_size = config.batchSize;
	int width = config.batchImgW;
	int height = config.batchImgH;

	if (config.rcorder == HFIRST) {
		width = config.batchImgH;
		height = config.batchImgW;
	}
	for (int i = 0; i < tem_batch_size; ++i) {
		cv::RNG rnger(cv::getTickCount());
		
		temMeta.index = 0;
		temMeta.img = cv::Mat(width, height, CV_8UC3);
		rnger.fill(temMeta.img, cv::RNG::UNIFORM, 0, 255);
	}

	auto shapeInput = tensorflow::TensorShape();
	auto tensorInput = Tensor(tensorflow::DT_FLOAT, shapeInput);

	Status temStatus = ImgPreProcess::cvtMat2TensorW({ &temMeta }, tensorInput);

	if (!temStatus.ok()) {
		LOG(ERROR) << "[TFSession] Mat->Tensor conversion failed: " << temStatus;
		return false;
	}
	std::vector<Tensor> resList;

	temStatus = session->Run({ { config.opsInput,tensorInput } },
		config.opsOutput,
		{}, &resList);

	if (!temStatus.ok()) {
		LOG(ERROR) << "[TFWorker] Running model failed: " << temStatus;
		return false;
	}

	printf("[TFSession] preating run is ok!\n");
	return true;
}

bool TFSession::isSessionOK()
{
	return sessionOK;
}

bool TFSession::run(TFCVMetaPtrVec& argImgVec, TFResVec& argResVec)
{
	argResVec.clear();
	std::vector<tensorflow::Tensor> resOutputs;
	if (WFIRST == config.rcorder) {
		auto flag_cvt = ImgPreProcess::cvtMat2TensorW(argImgVec, mTensorInput);
	}
	else {
		auto flag_cvt = ImgPreProcess::cvtMat2TensorH(argImgVec, mTensorInput);
	}

	auto status_run = session->Run(
		{ { config.opsInput,mTensorInput } }, 
		config.opsOutput,
		{}, &resOutputs);
	if (!status_run.ok()) {
		printf("[TFWorker] run session failed!\n");
		return false;
	}
	//run successed,get res to argResVec
	tfResEnqueue(resOutputs, argImgVec, argResVec);

	return true;
}

void TFSession::tfResEnqueue(std::vector<Tensor>& argRes, TFCVMetaPtrVec& argImgVec, TFResVec& argResVec)
{
	for (int i = 0; i < argRes.size(); ++i) { // number of output ops
		// row--batch index; col--result
		auto res_tensor_mapped = argRes[i].tensor<float, 1>();

		std::cout << "\n[TFWorker] output tensor: " 
			<< argRes[0].DebugString() << "   Index: " 
			<< argImgVec[0]->index << std::endl;
		/*std::cout << "\n[TFWorker] output tensor: " 
			<< argRes[0].DebugString() << "   Index: " 
			<< argMeta->index << "\n[TFWorker] output tensor: " 
			<< argRes[1].DebugString() << "   Index: " 
			<< argMeta->index << std::endl;*/

		for (int j = 0; j < argRes[i].dim_size(0); ++j) {
			TF_RES tem_res;
			tem_res.index = argImgVec[i]->index;
			tem_res.score = res_tensor_mapped(j);
			//tem_res.path = (*arg_batch)[j]->path;

			argResVec.push_back(tem_res);
			argImgVec[i]->img.release();
		}
	}
}
