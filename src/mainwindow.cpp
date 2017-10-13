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

#include "myhelper.h"
int STATICWAIT[] = {1500, 3300, 2500, 3300, 2500, 3300, 7000};

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
    pauseSeq << ui->c1 << ui->c2 << ui->c3<<  ui->c4 <<  ui->c5 <<  ui->c6 <<  ui->c7;

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
			return doWarning("The Angles are between -359~-1 and 1~359.");
	}
	else {
		if(hour >= ui->cycle->value()) {
			E_D_Status(true, false, false, true);
			return;
		}
		paused = 0;
		qDebug()<<"continue";
	}
    reset = 0;
    ui->stepLog->clear();

    logFile = new QFile("vw_knob_test_"  + QDateTime::currentDateTime().toString("yyyy-M-dd-hh-mm-ss") + ".txt");
    logFile->open(QIODevice::WriteOnly | QIODevice::Text);
    in = new QTextStream(logFile);
	E_D_Status(false, true, true, false);
	doAroll();
}

void MainWindow::freshSummary(int result)
{
  //  ui->lblTotalNbr  ui->cycle->value();
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
    //doWarning("Please waiting for about 5s to cancel the test.... ");
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

	if(min < angSeq.size()) {
		ang = angSeq[min]->text().toInt();
		if(paused == 0) {
            if(ang != 0) {

				if(min == 0) {
                    QString startLine ="Cycle " + QString::number(hour + 1) +  ": " + startTime + " Started";
                    ui->result->appendPlainText(startLine);/////
                    *in << startLine <<"\n";
                    //logFile->close();
				}
                else {
                    ui->result->moveCursor(QTextCursor::End);
                    ui->result->insertPlainText(lastArr);/////
                    ui->result->moveCursor(QTextCursor::End);
                    aLog+=lastArr;
                    *in<<aLog << "\n";
                }

				sw->buildPhaseCmd(ang);
                qDebug()<< STATICWAIT[min] << " + " << pauseSeq[min]->text().toInt() << " =" << STATICWAIT[min] + pauseSeq[min]->text().toInt();
                emit doPhaseCmd(SETCYC, STATICWAIT[min] + pauseSeq[min]->text().toInt());
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
       // QString finishLine = QDateTime::currentDateTime().toString("yyyy-M-dd,hh:mm:ss.zzz") + " cycle " + QString::number(hour) + " has finished";

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
                //QThread::sleep(3);
                ui->stepLog->clear();
                doAroll();
		}
		else {
                if(ui->start->isEnabled() == false && ui->pause->isEnabled() == false && ui->reset->isEnabled()==false) {
                    QList<QAbstractButton *> buttons = msgBox->buttons();
                    buttons[0]->setEnabled(true);
                    msgBox->setText("Please re-calibrate the device and contine");
                    E_D_Status(true, false, false, true);
                    return;
                }
            }
		}
		else {
			
            ui->result->appendPlainText("    Result :OK");
            ui->result->appendPlainText("");
            *in << "    Result : OK" << "\n";
            *in << "" << "\n";
            logFile->close();

			doWarning("The test has been finished");
            logFile->close();
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
