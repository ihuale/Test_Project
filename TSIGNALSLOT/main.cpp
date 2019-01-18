#include <QtCore/QCoreApplication>
#include <utility>
#include <QObject>
#include <QQueue>
#include <QDateTime>
#include <QSemaphore>
#include "Thread1.h"
#include "Thread2.h"
#include "CSBuffer.hpp"

QSemaphore semFree(100);
QSemaphore semUsed;

QQueue<std::pair<int, QDateTime>> que;
CSBuffer csbuffer(50, 25);


int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	Thread1 _thread1;
	Thread2 _thread2;

	/*QObject::connect(&_thread1, &Thread1::PrintMsg, 
		&_thread2, &Thread2::onPrintMsg);*/

	/*QObject::connect(&_thread1, &Thread1::PrintMsg,
		&_thread2, &Thread2::onChangeFlag);*/

	/*QObject::connect(&_thread1, &Thread1::finished,
		&_thread2, &Thread2::onTestFinished);*/

	_thread1.start();
	_thread2.start();
	//_thread1.start();


	return a.exec();
}
