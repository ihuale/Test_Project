#include "WSScan_ImageView.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	WSScan_ImageView w;
	w.show();
	return a.exec();
}
