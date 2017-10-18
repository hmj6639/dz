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
//int STATICWAIT[] = {1500, 3800, 2800, 3800, 2800, 3800, 7500};


int STATICWAIT[] = {1500, 3800, 2500, 3800, 2500, 3800, 6500};//slow
//int STATICWAIT[] = {1200, 2100, 1500, 2100, 1500, 2100, 3500};//fast
QString RESULTOK = "    Result : OK";

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
	sw = new SerialWorker(ui->Flog->isChecked());
	//sw->getAllDevice();
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
}

void MainWindow::run()
{
	serialWorkerThread.start();
	doWarning("Please make sure the following two actions before the test.\n\n 1: Tune the lever to the start point.\n 2: Turn the volumn to 0.");
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
	maxLogLine = 500000;
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

	for (int i = 0; i < doSeq.size(); i++) {
		if(doSeq[i]->isChecked() == true) {
			angs << angSeq[i]->text().toInt();
			durs << (STATICWAIT[i] + pauseSeq[i]->text().toInt());
		}
	}

	if(angs.size() == 0)
		return;
#if 1
	dir = new QString("vw_knob_test_" + QDateTime::currentDateTime().toString("yyyy-M-dd--hh-mm-ss") + "/");
	QDir().mkdir(*dir);
	if(logFileMotor == NULL && inMotor == NULL) {
		logFileMotor = new QFile(*dir + "Motor" + ".txt");
		logFileMotor->open(QIODevice::WriteOnly | QIODevice::Text);
		inMotor = new QTextStream(logFileMotor);
		for(int i = 0; i < 3; i++)
			fc[i] = 0;
	}
#endif

	E_D_Status(false, true, true, false);
	ui->pb->setRange(0, ui->cycle->value());
	ui->pb->setValue(0);
	ui->left->setText(QString::number(ui->cycle->value()));
	ui->lblTotalNbr->setText(QString::number(ui->cycle->value()));

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
	//hour = 0;
	//min = 0;
	count=0;

	reset = 1;
	QList<QAbstractButton *> buttons = msgBox->buttons();
	if(buttons.size() > 0)
		buttons[0]->setEnabled(false);
	doWarning("Warning: The test will be stopped until the current test finished!");
}

void MainWindow::fastGo(int ang)
{
	if(ang != 0 && hour == 0 && min == 0 && isGo == 0) {
		isGo = 1;
		sw->buildPhaseCmd(ang);
		emit doPhaseCmd(SETCYC, 1000);
	}
	else {
		qDebug()<<ang << hour << min << isGo;
		doWarning("Please stop the Test");
	}
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

	if(min < angs.size()) {
		ang = angs[min];
		if(paused == 0) {
			if(ang != 0) {

				if(min == 0) {
					QString startLine ="Cycle " + QString::number(hour + 1) + ": " + startTime + " Started";
					ui->result->appendPlainText(startLine);/////
					*inMotor << startLine <<"\n";
					{
						if(hour == 0) {
							logFile[0] = new QFile(*dir + "Product_A_" + "_shift" + ".txt");
							logFile[0]->open(QIODevice::WriteOnly | QIODevice::Text);
							in[0] = new QTextStream(logFile[0]);

							logFile[1] = new QFile(*dir + "Product_B_" + "_shift" + ".txt");
							logFile[1]->open(QIODevice::WriteOnly | QIODevice::Text);
							in[1] = new QTextStream(logFile[1]);

							logFile[2] = new QFile(*dir + "Product_C_" + "_shift" + ".txt");
							logFile[2]->open(QIODevice::WriteOnly | QIODevice::Text);
							in[2] = new QTextStream(logFile[2]);
						}

						if(in[0] && logFile[0])
							*in[0] << startLine << "\n";
						if(in[1] && logFile[1])
							*in[1] << startLine << "\n";
						if(in[2] && logFile[2])
							*in[2] << startLine << "\n";
					}
				}
				else {
					ui->result->moveCursor(QTextCursor::End);
					ui->result->insertPlainText(lastArr);/////
					ui->result->moveCursor(QTextCursor::End);
					aLog+=lastArr;
					*inMotor<<aLog << "\n";
				}
				inMotor->flush();

				sw->buildPhaseCmd(ang);
				qDebug()<< durs[min];
				emit doPhaseCmd(SETCYC, durs[min]);
				startTime = QDateTime::currentDateTime().toString("yyyy-M-dd,hh:mm:ss.zzz");
				QString angLine = "    " + startTime + " roll " + QString::number(ang);
				if(min == 0 || min == angs.size() -2)
					angLine += " and press";
				if(min == angs.size() -1)
					angLine += " and return to start point";
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

		if(hour % 200 == 0)
			ui->result->clear();

		if(hour < ui->cycle->value()) {
			if(reset == 0) {
				ui->result->moveCursor(QTextCursor::End);
				ui->result->insertPlainText(lastArr);
				ui->result->moveCursor(QTextCursor::End);
				aLog+=lastArr;
				*inMotor<<aLog << "\n";

				ui->result->appendPlainText(RESULTOK);
				ui->result->appendPlainText("");
				*inMotor << RESULTOK << "\n";
				*inMotor << "" << "\n";
				inMotor->flush();
				count++;
				ui->left->setText(QString::number(ui->cycle->value() - count));
				ui->pb->setValue(count);
				doAroll();
			}
			else {
				if(ui->start->isEnabled() == false && ui->pause->isEnabled() == false && ui->reset->isEnabled()==false) {
					QList<QAbstractButton *> buttons = msgBox->buttons();
					if(buttons.size() > 0)
						buttons[0]->setEnabled(true);
					msgBox->setText("Please re-calibrate the device and continue");
					E_D_Status(true, false, false, true);
				}
				reset = 0;
				hour = 0;
				ui->result->appendPlainText(RESULTOK);
				ui->result->appendPlainText("");
				*inMotor << RESULTOK << "\n";
				*inMotor << "" << "\n";
				logFileMotor->close();
				return;
			}
		}
		else {

			ui->result->appendPlainText(RESULTOK);
			ui->result->appendPlainText("");
			*inMotor << RESULTOK << "\n";
			*inMotor << "" << "\n";
			logFileMotor->close();
			count++;
			ui->left->setText(0);
			ui->pb->setValue(count);
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

void MainWindow::updateCount(int pid, int type, int current, int acc)
{
	static int last[3] = {0, 0, 0};
	QString pre;

	if(dir == NULL)
		return;

	if(logFile[pid] == NULL && in[pid] == NULL) {

		if(pid == 0)
			pre = "Product_A_";
		else if(pid == 1)
			pre = "Product_B_";
		else
			pre = "Product_C_";
	}

	if(type < 0 && in[pid]) {
		*in[pid] << QString::number(last[pid]) << "\n";
		in[pid]->flush();
	}
	last[pid] = acc;
}

void MainWindow::updateSerialLog(int pid, QByteArray t)
{
	QString pre;

	if(dir == NULL)
		return;

	if(pid == 0)
		pre = "Product_A_";
	else if(pid == 1)
		pre = "Product_B_";
	else
		pre = "Product_C_";

	if(lc[pid] >= maxLogLine) {
		logFilef[pid]->close();
		fc[pid]++;
		if(logFilef[pid] != NULL) {
			delete logFilef[pid];
			logFilef[pid] = NULL;
		}

		if(inf[pid] != NULL) {
			delete inf[pid];
			inf[pid] = NULL;
		}
	}

	if(logFilef[pid] == NULL && inf[pid] == NULL) {
		logFilef[pid] = new QFile(*dir + pre + "_full_" + QString::number(fc[pid]) + ".txt");
		logFilef[pid]->open(QIODevice::WriteOnly | QIODevice::Text);
		inf[pid] = new QTextStream(logFilef[pid]);
		lc[pid] = 0;
	}

	if(paused ==0 && reset ==0 && isGo == 0) {
		*inf[pid] << t;
		inf[pid]->flush();
		lc[pid]++;
	}
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

void MainWindow::on_pushButton_clicked()
{
	ui->left->setText("");
}
