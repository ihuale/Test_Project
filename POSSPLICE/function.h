#pragma once
#include <queue>
#include "IniFile.h"
#include "AIType.h"
#define NOMINMAX

extern HL::SpliceInfo info;
extern HL::DataInfo datainfo;


int config();

bool readPosInfo();

bool calcPosRect();

bool calcPosOffset();

bool splice();

void readImg(int arg_index);

void getWriteDataJob();

void doWriteDataJob(int arg_index);

bool pyramid();