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
    w.doWarning("Please calibrate the device first");

	return a.exec();
}
