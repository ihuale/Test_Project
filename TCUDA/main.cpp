#include <iostream>
#include "test_func.cuh"


int main()
{
	int width = 1 << 10;
	int height = 1 << 10;
	Matrix *A, *B, *C;

	cudaMallocManaged((void**)&A, sizeof(Matrix));
	cudaMallocManaged((void**)&B, sizeof(Matrix));
	cudaMallocManaged((void**)&C, sizeof(Matrix));
	int nBytes = width * height * sizeof(float);
	cudaMallocManaged((void**)&A->elements, nBytes);
	cudaMallocManaged((void**)&B->elements, nBytes);
	cudaMallocManaged((void**)&C->elements, nBytes);


	A->height = height;
	A->width = width;
	B->height = height;
	B->width = width;
	C->height = height;
	C->width = width;
	for (int i = 0; i < width * height; ++i)
	{
		A->elements[i] = 1.0;
		B->elements[i] = 2.0;
	}

	dim3 blockSize(32, 32);
	dim3 gridSize((width + blockSize.x - 1) / blockSize.x,
		(height + blockSize.y - 1) / blockSize.y);
	// Ö´ÐÐkernel
	matMulKernel << < gridSize, blockSize >> > (A, B, C);

	cudaDeviceSynchronize();
	float maxError = 0.0;
	for (int i = 0; i < width * height; ++i)
		maxError = fmax(maxError, fabs(C->elements[i] - 2 * width));
	std::cout << "×î´óÎó²î: " << maxError << std::endl;

	return 0;
}