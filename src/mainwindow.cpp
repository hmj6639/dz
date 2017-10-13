#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QFontDatabase>
#include <QDebug>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>

#include "myhelper.h"
int STATICWAIT[] = {1500, 3800, 2800, 3800, 2800, 3800, 7500};

void MainWindow::initSize()
{
	const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);

	QFont font;
	int fontId = QFontDatabase::addApplicationFont(":/res/fonts/LucidaTypewriterRegular.ttf");
	if (fontId != -1) {
		const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
		if (!families.empty()) {
			font.setFamily(families.at(0));
			font.setPointSize(10);
		}
	} else {
		font.setFamily(QStringLiteral("Courier"));
		font.setPointSize(10);
	}

	qApp->setFont(font);

	//int h = availableGeometry.height() * 3 / 4;
	//int w = h * 850 / 600;
	int h = availableGeometry.height() * 3 / 4;
	int w = h * 16 / 10;
	resize(w, h);
	setIconSize(QSize(16, 16));
	msgBox = new QMessageBox(this);
	msgBox->setWindowTitle("Warning");

	angSeq << ui->a1 << ui->a2 << ui->a3 << ui->a4 << ui->a5 << ui->a6 << ui->a7;
	pauseSeq << ui->c1 << ui->c2 << ui->c3<< ui->c4 << ui->c5 << ui->c6 << ui->c7;
	doSeq << ui->k1 << ui->k2 << ui->k3 << ui->k4 << ui->k5 << ui->k6 << ui->k7;

	openDevice();
	return;
}

void MainWindow::openDevice()
{
	sw = new SerialWorker();
	sw->moveToThread(&serialWorkerThread);
	connect(&serialWorkerThread, &QThread::started, sw, &SerialWorker::run);
	connect(this, &MainWindow::sendRawData, sw, &SerialWorker::sendRawData);
	connect(this, &MainWindow::doPhaseCmd, sw, &SerialWorker::doPhaseCmd);
	////connect(sw, &SerialWorker::processNav, this, &MainWindow::processNavRes);
	connect(sw, &SerialWorker::rollfinish, this, &MainWindow::rollfinish);
	connect(sw, &SerialWorker::updateVol, this, &MainWindow::updateVol);
    connect(sw, &SerialWorker::updateCount, this, &MainWindow::updateCount);
    connect(sw, &SerialWorker::updateSerialLog, this, &MainWindow::updateSerialLog);
	connect(&serialWorkerThread, &QThread::finished, sw, &QObject::deleteLater);
	qDebug()<<"open worker";
	serialWorkerThread.start();
}

void MainWindow::rollfinish()
{
	if(isGo == 0)
		doAroll();
	else
		isGo = 0;
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	initSize();
}

MainWindow::~MainWindow()
{
	delete ui;
	serialWorkerThread.quit();
	serialWorkerThread.wait();
}

void MainWindow::on_start_clicked()
{
	if(paused == 0) {
		hour = 0;
		min = 0;
		if(checkAng() != 0)
			return doWarning("The Angles are between 1 and 359.");
	}
	else {
		if(hour >= ui->cycle->value()) {
			E_D_Status(true, false, false, true);
			return;
		}
		paused = 0;
		qDebug()<<"continue";
		E_D_Status(false, true, true, false);
		doAroll();
		return;
	}
	reset = 0;

	angs.clear();
	durs.clear();

	for (int i = 0; i < doSeq.size(); i ++) {
		if(doSeq[i]->isChecked() == true) {
			angs << angSeq[i]->text().toInt();
			durs << (STATICWAIT[i] + pauseSeq[i]->text().toInt());
		}
	}

	if(angs.size() == 0)
		return;

    if(logFile == NULL && in == NULL) {
        logFile = new QFile("vw_knob_test_" + QDateTime::currentDateTime().toString("yyyy-M-dd-hh-mm-ss") + ".txt");
        logFile->open(QIODevice::WriteOnly | QIODevice::Text);
        in = new QTextStream(logFile);
    }

	E_D_Status(false, true, true, false);

	doAroll();
}

void MainWindow::freshSummary(int result)
{
	if(result == 1) {

	}
	else if(result == -1) {
	}
}

int MainWindow::checkAng()
{
	return 0;
	int total = 0;

	for(int i = 0; i < angSeq.size(); i++) {
		int ang = angSeq[i]->text().toInt();
		/*if(ang == 0 || ang > 359 || ang < -359)
		  return 1;*/
		total+=ang;
	}

	if(total % 360 != 0)
		return 1;
	else
		return 0;
}

void MainWindow::doWarning(QString label)
{
	myHelper::ShowMessageBoxInfo(label);
	//msgBox->setText(label);
	//msgBox->exec();
}

void MainWindow::E_D_Status(bool s, bool p, bool r, bool c)
{
	ui->start->setEnabled(s);
	ui->pause->setEnabled(p);
	ui->reset->setEnabled(r);
	ui->cycle->setEnabled(c);
}

void MainWindow::on_pause_clicked()
{
	E_D_Status(true, false, true, true);
	paused = 1;
}

void MainWindow::on_reset_clicked()
{
	E_D_Status(false, false, false, false);
	paused = 0;
	hour = 0;
	min = 0;

	reset = 1;
	QList<QAbstractButton *> buttons = msgBox->buttons();
	if(buttons.size() > 0)
		buttons[0]->setEnabled(false);
	doWarning(QString::number(buttons.size()) + "Warning: The test will be stopped until the current test finished!");
}

void MainWindow::fastGo(int ang)
{
	if(ang != 0 && hour == 0 && min == 0 && isGo == 0) {
		isGo = 1;
		sw->buildPhaseCmd(ang);
		emit doPhaseCmd(SETCYC, 800);
	}
	else
		doWarning("Please stop the Test");
}

void MainWindow::on_btnAdClockwise_clicked()
{
	int ang = ui->trim->text().toInt();

	fastGo(ang);
}

void MainWindow::on_Go_clicked()
{
	int ang = ui->trim->text().toInt();

	fastGo(0 - ang);
}

void MainWindow::doAroll()
{
	int ang;
	QString startTime = QDateTime::currentDateTime().toString("yyyy-M-dd, hh:mm:ss.zzz");
	QString lastArr = "   -->ok";
	static QString aLog;

	if(reset == 1) {
		if(ui->start->isEnabled() == false && ui->pause->isEnabled() == false && ui->reset->isEnabled()==false) {
			QList<QAbstractButton *> buttons = msgBox->buttons();
			if(buttons.size() > 0)
				buttons[0]->setEnabled(true);
			msgBox->setText("Please re-calibrate the device and continue");
			E_D_Status(true, false, false, true);
		}
		reset = 0;
		ui->result->appendPlainText("    Result :OK");
		ui->result->appendPlainText("");
		*in << "    Result : OK" << "\n";
		*in << "" << "\n";
		logFile->close();
		return;
	}

	if(min < angs.size()) {
		ang = angs[min];
		if(paused == 0) {
			if(ang != 0) {

				if(min == 0) {
					QString startLine ="Cycle " + QString::number(hour + 1) + ": " + startTime + " Started";
					ui->result->appendPlainText(startLine);/////
					*in << startLine <<"\n";
				}
				else {
					ui->result->moveCursor(QTextCursor::End);
					ui->result->insertPlainText(lastArr);/////
					ui->result->moveCursor(QTextCursor::End);
					aLog+=lastArr;
					*in<<aLog << "\n";
				}

				sw->buildPhaseCmd(ang);
				qDebug()<< durs[min];
				emit doPhaseCmd(SETCYC, durs[min]);
				startTime = QDateTime::currentDateTime().toString("yyyy-M-dd,hh:mm:ss.zzz");
				QString angLine = "    " + startTime + " roll " + QString::number(ang);
				ui->result->appendPlainText(angLine);
				aLog = angLine;

				min++;
			}
			else {
				min++;
				doAroll();
			}
		}
	}
	else {
		hour++;
		min = 0;

		if(hour < ui->cycle->value()) {
			if(reset == 0) {
				ui->result->moveCursor(QTextCursor::End);
				ui->result->insertPlainText(lastArr);
				ui->result->moveCursor(QTextCursor::End);
				aLog+=lastArr;
				*in<<aLog << "\n";

				ui->result->appendPlainText("    Result :OK");
				ui->result->appendPlainText("");
				*in << "    Result : OK" << "\n";
				*in << "" << "\n";
				doAroll();
			}
		}
		else {

			ui->result->appendPlainText("    Result :OK");
			ui->result->appendPlainText("");
			*in << "    Result : OK" << "\n";
			*in << "" << "\n";
			logFile->close();

			doWarning("The test has been finished");
			E_D_Status(true, false, false, true);
			min = 0;
			hour = 0;
		}
		return;
	}
}

void MainWindow::updateVol(int pid, int vol)
{
    qDebug()<<"now min " << min << " product " << pid << " vol " << vol;
}

void MainWindow::updateCount(int type, int current, int acc)
{
    static int last;
    if(logFile == NULL && in == NULL) {
        logFile = new QFile("vw_knob_test_" + QDateTime::currentDateTime().toString("yyyy-M-dd-hh-mm-ss") + ".txt");
        logFile->open(QIODevice::WriteOnly | QIODevice::Text);
        in = new QTextStream(logFile);
    }

    ui->result->appendPlainText("rotation count is " + QString::number(current) + "  ---   "  +  QString::number(acc));
    if(type < 0 && in) {
        *in << QString::number(last) << "\n";
        in->flush();
    }
    last = acc;
}

void MainWindow::updateSerialLog(QByteArray t)
{
    if(logFilef == NULL && inf == NULL) {
        logFilef = new QFile("vw_knob_test_full_" + QDateTime::currentDateTime().toString("yyyy-M-dd-hh-mm-ss") + ".txt");
        logFilef->open(QIODevice::WriteOnly | QIODevice::Text);
        inf = new QTextStream(logFilef);
    }
     *inf << t;
    inf->flush();
}

void MainWindow::modeSwitch(int /*mode*/)
{
	/*ui->tv->setModel(dataModel[mode]);

	  for(int i = 0; i < 9; i++)
	  ui->tv->setColumnWidth(i, cwidth[i]);*/
}

void MainWindow::on_all_clicked()
{
	modeSwitch(0);
}

void MainWindow::on_ok_clicked()
{
	modeSwitch(2);
}

void MainWindow::on_nok_clicked()
{
	modeSwitch(1);
}

void MainWindow::on_Flog_toggled(bool checked)
{
    qDebug()<<checked;
    sw->setFulllog(checked);
}
