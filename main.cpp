#include "mainwindow.h"
#include <QApplication>
#include "myhelper.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	myHelper::SetStyle("navy");

	MainWindow w;
	w.show();
	//w.showMaximized();
//	w.doWarning("Please make sure the following two actions before the test.\n\n 1: Tune the lever to the start point.\n 2: Turn the volumn to 0.");

	w.run();
	return a.exec();
}
