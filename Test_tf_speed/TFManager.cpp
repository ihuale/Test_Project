#include "TFManager.h"


TFManager::TFManager() :
	abort_(false)
{
	options_ = new tensorflow::SessionOptions();
	tfworker_ = new TFManager::TFWorker();
}


TFManager::~TFManager()
{
	tfworker_->setWorkState(false);
}

bool TFManager::getImageBatch(MatList& res, unsigned int batch)
{
	if (tfworker_) {
		return tfworker_->getImageBatch(res, batch);
	}
	return false;
}

bool TFManager::addImage(MatPair* arg)
{
	if (tfworker_) {
		return tfworker_->addImage(arg);
	}
	return false;
}

bool TFManager::addImageBatch(MatList& arg)
{
	if (tfworker_) {
		return tfworker_->addImageBatch(arg);
	}
	return false;
}

tensorflow::SessionOptions* TFManager::mutable_session_options()
{
	if (options_ == NULL) {
		options_ = new tensorflow::SessionOptions();
	}
	return options_;
}

bool TFManager::initializeTF()
{
	if (!tfworker_) {
		std::cout << "TFManager::initializeTF(): tensorflow worker has not initialized!" 
			<< std::endl;
		return false;
	}
	if (!tfworker_->isGraphdefInitialize()) {
		std::cout << "TFManager::initializeTF(): TFWorker graphdef has not initialized!" 
			<< std::endl;
		return false;
	}
	if (!options_) {
		options_ = new tensorflow::SessionOptions();
	}
	auto status_creat = tfworker_->initializeSession(*options_);
	if (!status_creat.ok()) {
		std::cout << "TFManager::initializeTF(): Creat session has fialed!" 
			<< std::endl;
		return false;
	}
}

void TFManager::setWorkState(bool arg)
{
	if (!tfworker_) {
		std::cout << "tensorflow has no initialized!" << std::endl;
		return;
	}
	tfworker_->setWorkState(arg);
}

void TFManager::prediction()
{
	if (!tfworker_) {
		std::cout << "tensorflow has no initialized!" << std::endl;
		return;
	}
	tfworker_->prediction();
}

void TFManager::getAllResult(ResultList& res)
{
	if (!tfworker_) {
		std::cout << "tensorflow has no initialized!" << std::endl;
		return;
	}
	tfworker_->getAllResult(res);
}
