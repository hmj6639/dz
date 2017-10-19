#include "mainwindow.h"
#include <QApplication>
#include "myhelper.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	myHelper::SetStyle("navy");

	MainWindow w;
	w.show();

	w.run();
	return a.exec();
}
