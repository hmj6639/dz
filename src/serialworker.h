#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QMutex>
#include <QSettings>

#define MAXCOM 4
#define COM_PHASE 0

typedef struct {
	QSerialPort *serial = NULL;
	QByteArray res;
	unsigned char  ffFlag = 0;
}Serial_FD;

enum phase{
	ENABLEPHASE = 0,
	SETTYPE,
	SETCYC,
	SETANG,
	STARTR,
	STOPR,
	CHECKARR,
	OVERROLL
};

class SerialWorker : public QObject
{
	Q_OBJECT

public:
	explicit SerialWorker(int, QObject *parent = 0);
	~SerialWorker();
	void getAllDevice(QStringList &listDev);
	void run();
	void buildPhaseCmd(int gui_ang);
	void setFulllog(bool n) {isFull = n;}
	void closeDevices();

public slots:
	void doPhaseCmd(int type, int w);
	void openProduct(QStringList dev);
    void controlVol();

signals:
	void processMoto(QString);
	void processNav(int, QString);
	void rollfinish();
	void updateVol(int, int, QByteArray);
	void updateCount(int, int, int, int);
	void updateSerialLog(int, QByteArray);
    void informPress(int);

private slots:
	void readSerialData();
	void phaseRes();

private:
	void setSerialPort(int, QSerialPortInfo*,int);
	void openDevice(int, QString tryDev,int);
	void closeDevice(int);

	void dealWithPhaseRes(QByteArray&);
	void dealWithNavRes(int, QByteArray &);
	void dealWithNavSerial(int, QByteArray &);
	void refind(int, QByteArray&);
	int checkPhaseRes();
	void procRXChar(int sid, unsigned char c);
	void handleFullData(int sid);
	void procVol(int sid, int pid, QByteArray);
	void openMotor();

	Serial_FD sfd[MAXCOM];
	unsigned char *cmd;
	int sec;
	unsigned char tmpRes[32];
	bool isFull = false;
	int motorReady = 0;
  //  QString p[3] = {"","",""};
    QString r1 = "";
    QString r2 = "";
    QString r3 = "";
};

#endif // SERIALWORKER_H
