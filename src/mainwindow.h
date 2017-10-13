#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "serialworker.h"

class QLineEdit;
class QCheckBox;
class QStandardItemModel;
class QMessageBox;


namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	void doWarning(QString label);

private slots:

	void on_start_clicked();
	void on_reset_clicked();
	void on_all_clicked();
	void on_ok_clicked();
	void on_nok_clicked();
	void modeSwitch(int);
	void on_pause_clicked();
	void on_btnAdClockwise_clicked();
	void on_Go_clicked();

private slots:
    void on_btnMenu_Close_clicked();
    void on_btnMenu_Max_clicked();
    void on_btnMenu_Min_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *);

private:
    void InitStyle();

    QPoint mousePoint;
    bool mousePressed;
    bool max;
    QRect location;

private:
    void initSize();

	Ui::MainWindow *ui;
	QMessageBox *msgBox;
	int hour = 0, min = 0;
	int paused = 0, isGo = 0;
    int reset = 0;

	SerialWorker *sw = NULL;
	QThread serialWorkerThread;
	QList <QLineEdit *>angSeq;
    QList <QLineEdit *>pauseSeq;

	int checkAng();
	void E_D_Status(bool, bool, bool, bool);
	void fastGo(int);
	void openDevice();
	void freshSummary(int result);

public 	slots:
	void doAroll();
	void rollfinish();
	void updateVol(int, int);

signals:
	void sendRawData(int, QString);
	void doPhaseCmd(int, int);
};

#endif // MAINWINDOW_H
