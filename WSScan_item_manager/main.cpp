#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include "TF/test_object_dection.h"
#include "TF/test_link_two_pb.h"

#ifndef COMPILER_MSVC
#define COMPILER_MSVC
#endif //COMPILER_MSVC

#ifndef NOMINMAX
#define NOMINMAX
#endif //NOMINMAX

#include <numeric>
#include <fstream>
#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include <eigen/Dense>
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/tensor_shape.h"

#include <opencv2/opencv.hpp>

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QDir>
#include <QtConcurrent>
#include <QThread>

#include "TF/types.h"

using tensorflow::TensorShape;
using tensorflow::Tensor;
using tensorflow::Scope;

//std::string path_image_read = "E:/data/test/2018-10-14-104136-048/";
std::string path_image_read = "E:/data/test/";

std::string image_filter = ".jpg";
//std::string image_filter = ".tif";

int main(int argc, char *argv[])
{
	//QApplication a(argc, argv);

	//test__run_object_dection();
	//test_link_two_pb();
	//QtConcurrent::run(&test_eight_classifier_pb);
	//QtConcurrent::run(&tenst_compare_pb);

	
	/*MainWindow w;
	w.show();
	return a.exec();*/
	
	int data = 0;
	Tensor a(tensorflow::DT_FLOAT, TensorShape({ 2,3,2,4 }));
	std::cout << "[main] before a.DebugString():" << a.DebugString() << std::endl;
	auto a_res = a.tensor<float, 4>();
	for (int i = 0; i < a.dim_size(0); ++i) {
		for (int ii = 0; ii < a.dim_size(1); ++ii) {
			for (int iii = 0; iii < a.dim_size(2); ++iii) {
				for (int iiii = 0; iiii < a.dim_size(3); ++iiii) {
					a_res(i, ii, iii, iiii) = data++;
				}
			}
		}
	}
	std::cout << "[main] after shaped(), a.DebugString():" << a.DebugString() << std::endl;
	std::cout << "[main] (0,0,0,1):" << a_res(0, 0, 0, 1) << std::endl;
	std::cout << "[main] (0,0,1,0):" << a_res(0, 0, 1, 0) << std::endl;

	auto flat = a.shaped<float, 3>({ 2,3,8 });
	float* tem_flat_tensor = flat.data();
	for (int i = 0; i < 16; ++i) {
		std::cout << i << "    ";
	}
	std::cout << std::endl;
	for (int i = 0; i < 16; ++i) {
		std::cout << *(tem_flat_tensor + i) << "    ";
	}
	std::cout << std::endl;

	system("pause");
	return 0;
}
