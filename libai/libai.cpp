// libai.cpp: define dll application export function
#include <mutex>
#include "libai.h"
#include "TF.h"

class AIContex {
public:
	TFSession* pTFSession;
};

AIHandle AIinit(AIConfig* cfg)
{
	AIContex* pctx = new AIContex;
	pctx->pTFSession = new TFSession;
	pctx->pTFSession->setGraphPath(cfg->model_path);
	pctx->pTFSession->config = {
		cfg->opsInput,
		cfg->opsOutput,
		cfg->batchSize,
		cfg->batchImgW,
		cfg->batchImgH,
		cfg->batchImgChannels,
		cfg->rotate,
		cfg->pxorder,
		cfg->rgborder
	};

	auto flag = pctx->pTFSession->preatingRun();

	if (flag && pctx->pTFSession->isSessionOK()) {
		//config tf successed
		return (AIHandle)pctx;
	}
	else {
		//config tf failed
		delete pctx->pTFSession;
		delete pctx;
		return 0;
	}
}

void AIfree(AIHandle handle)
{
	if (0 == handle)
		return; 
	AIContex* pctx = (AIContex*)handle;

	delete pctx->pTFSession;

	delete pctx;
}

bool AIrun(AIHandle handle, TFCVMetaPtrVec& argImgVec, TFResVec& argResVec)
{
	if (handle == 0)
		return false;

	AIContex* pctx = (AIContex*)handle;

	if (!pctx->pTFSession->isSessionOK()) {
		printf("[LIBAI] tfsession is not ok! please init first\n");
		return false;
	}
	return pctx->pTFSession->run(argImgVec, argResVec);
}
