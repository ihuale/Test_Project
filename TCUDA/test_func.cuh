#pragma once
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

struct Matrix
{
	int width;
	int height;
	float *elements;
};

__device__ float getElement(Matrix *A, int row, int col)
{
	return A->elements[row * A->width + col];
}

__device__ void setElement(Matrix *A, int row, int col, float value)
{
	A->elements[row * A->width + col] = value;
}

__global__ void matMulKernel(Matrix *A, Matrix *B, Matrix *C)
{
	float Cvalue = 0.0;
	int row = threadIdx.y + blockIdx.y * blockDim.y;
	int col = threadIdx.x + blockIdx.x * blockDim.x;
	for (int i = 0; i < A->width; ++i)
	{
		Cvalue += getElement(A, row, i) * getElement(B, i, col);
	}
	setElement(C, row, col, Cvalue);
}