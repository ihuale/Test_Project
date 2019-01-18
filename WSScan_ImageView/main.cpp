#define CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  

#include "WSScanMW.h"
#include <QtWidgets/QApplication>

#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif //_DEBUG

inline void EnableMemLeakCheck()
{
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
}

int main(int argc, char *argv[])
{
	//EnableMemLeakCheck();
	QApplication a(argc, argv);
	WSScanMW w;
	w.show();
	//return a.exec();
	int tem = a.exec();
	//_CrtDumpMemoryLeaks();
	return tem;
}
