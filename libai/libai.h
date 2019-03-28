#pragma once
#ifdef LIBAI_EXPORTS
#define LIBAI_API extern "C" __declspec(dllexport)
#else
#define LIBAI_API extern "C" __declspec(dllimport)
#endif // !LIBAI_EXPORTS

//libai usage:
//first, AIinit
//if successed, then will get handel on cfg
//and then,use AIrun to process the data
//use AIfree to release handle before app exit
//
//note: if AIinit failed, AIrun func is not accessed

#include "AIType.h"

//if init failed, 
//not allowed access all other func!!!

//return 0 if init failed
//return handle if successed
LIBAI_API AIHandle AIinit(AIConfig* cfg);
LIBAI_API void AIfree(AIHandle handle);

//return true: calc over, res on argResVec,release all argImgVec
//return false: calc failed,no release data
LIBAI_API bool AIrun(AIHandle handle, TFCVMetaPtrVec& argImgVec, TFResVec& argResVec);
