//#include <QtWidgets/QApplication>
//#include <QMainWindow>
//#include <QWidget>
//#include <QLabel>
//
//#include <cstdlib>//for rand()
//#include <utility>
//#include <vector>
//#include <iostream>
//
//#ifdef _DEBUG
//#include <ctime>
//#endif //_DEBUG
//
//#include "TileMap.h"
//
//#ifndef COMPILER_MSVC
//#define COMPILER_MSVC
//#endif //COMPILER_MSVC
//
//#ifndef NOMINMAX
//#define NOMINMAX
//#endif //NOMINMAX
//
//int main(int argc, char *argv[])
//{
//#ifdef _DEBUG
//	time_t tem_start = clock();
//#endif //_DEBUG
//	std::cout << "arg list:\ncol row" << std::endl;
//	QApplication a(argc, argv);
//
//	std::vector<int> size_arry;
//	if (argc == 3) {
//		for (int tem_index = 1; tem_index < argc; ++tem_index) {
//			int tem_num = 0;
//			sscanf(argv[tem_index], "%d", &tem_num);
//			if (tem_num == 0) {
//				tem_num = 20;
//			}
//			size_arry.push_back(tem_num);
//			std::cout << argv[tem_index] << "---" << tem_num << std::endl;
//		}
//	}
//	else if (argc == 2) {
//		int tem_num = 0;
//		sscanf(argv[1], "%d", &tem_num);
//		if (tem_num == 0) {
//			tem_num = 20;
//		}
//		size_arry.push_back(tem_num);
//		size_arry.push_back(20);
//	}
//	else {
//		size_arry.push_back(20);
//		size_arry.push_back(20);
//	}
//
//	QWidget win_widget;
//	//win_widget.resize(400, 300);
//	win_widget.setFixedSize(size_arry[0] * 10 + 2, size_arry[1] * 5 + 2);
//
//	TileMap m_label(&win_widget, size_arry[0], size_arry[1]);
//	//m_label.setGeometry(10, 10, 300, 200);
//	//m_label.setDirection(TileMap::ColFirst);
//	if (argc > 1) {
//		/*for (int i = 0; i < size_arry[0] * size_arry[1]; ++i) {
//			m_label.add_result(std::make_pair(i,
//				std::make_pair(rand() % 100,
//					rand() % 3)));
//		}*/
//	}
//
//	m_label.setFocus(Qt::FocusReason::OtherFocusReason);
//	win_widget.show();
//
//#ifdef _DEBUG
//	time_t tem_end_4 = clock();
//	std::cout << "Time 4:"
//		<< difftime(tem_end_4, tem_start)
//		<< std::endl;
//#endif //_DEBUG
//
//	return a.exec();
//}