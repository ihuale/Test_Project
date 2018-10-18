#include "head.h"

int Classification(double arg_score)
{
	if (arg_score < 0 || arg_score>1) {
		printf("[Classification] ERROR ind arg_score: %lf\n", arg_score);
		return -1;
	}
	if (arg_score < ClassRatioMap[0]) {
		return 0;
	}
	else if (arg_score < ClassRatioMap[1]) {
		return 1;
	}
	else if (arg_score < ClassRatioMap[2]) {
		return 2;
	}
	else if (arg_score <= ClassRatioMap[3]) {
		return 3;
	}

	//in case
	return -1;
}
