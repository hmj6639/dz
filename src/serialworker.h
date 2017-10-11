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
	int req_ = 0;
	QByteArray res;
	unsigned char  ffFlag;
}Serial_FD;

enum phase{
	ENABLEPHASE = 0,
	SETTYPE,
	SETCYC,
	SETANG,
	STARTPHASE,
	FININSHROLL,
	OVERROLL
};

class SerialWorker : public QObject
{
	Q_OBJECT

public:
	explicit SerialWorker( QObject *parent = 0);
	~SerialWorker();
	void getAllDevice(QStringList &listDev);
	void run();
	void buildPhaseCmd(int gui_ang);

public slots:
	void sendRawData(int, const QString raw);
	void doPhaseCmd(int type, int w);

signals:
	void processMoto(QString res);
	void processNav(int, QString res);
	void rollfinish();
	void updateVol(int, int);

private slots:
	void readSerialData();
	void phaseRes();

private:
	void setSerialPort(int, QSerialPortInfo*,int);
	void openDevice(int, QString tryDev,int);
	void closeDevice(int);

	void dealWithPhaseRes(QByteArray&);
	void dealWithNavRes(int, QByteArray &);
	int checkPhaseRes();
	void procRXChar(int sid, unsigned char c);
	void handleFullData(int sid);
	void procVol(int pid, QByteArray res);

	Serial_FD sfd[MAXCOM];
	unsigned char *cmd;
	int sec;
	unsigned char tmpRes[32];
};

#endif // SERIALWORKER_H
