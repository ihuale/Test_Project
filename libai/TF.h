#pragma once
#include <memory>
#include <mutex>
#include <thread>
#include "AIType.h"
#include "TFIncludeh.h"

//warpper of tf::session
class TFSession
{
public:
	explicit TFSession();
	~TFSession();

	struct SessionConfig {
		std::string opsInput;
		std::vector<std::string> opsOutput;

		int batchSize;
		int batchImgW;
		int batchImgH;
		int batchImgChannels;

		//this for clockwise
		int rotate;

		RCOrder rcorder;
		rgbOrder rgbOrder;
	};

	std::unique_ptr<tensorflow::Session> session;
	SessionConfig config;

	bool setGraphPath(std::string arg_pbPath);
	bool preatingRun();
	bool isSessionOK();

public:
	//for dll loader
	bool run(TFCVMetaPtrVec& argImgVec, TFResVec& argResVec);

private:
	void tfResEnqueue(std::vector<Tensor>& argRes, 
		TFCVMetaPtrVec& argImgVec, TFResVec& argResVec);

private:
	std::string  m_pbPath;
	volatile bool sessionOK;

	tensorflow::TensorShape mShapeInput;
	tensorflow::Tensor mTensorInput;
};
