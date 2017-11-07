#include <QDebug>
#include <QThread>
#include "serialworker.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MOTOR_SPEED 38400
#define PRODUCT_SPEED 115200

unsigned char bufEng[] = {0x01, 0x10, 0x71, 0x48, 0x00, 0x02, 0x04, 0x00, 0x4f, 0x00, 0x00, 0xaf, 0xbc};
unsigned char phaseEngRes[] = {0x01, 0x10, 0x71, 0x48, 0x00, 0x02, 0xda, 0xe2};
unsigned char bufPos[] = {0x01, 0x10, 0x71, 0x58, 0x00, 0x02, 0x04, 0x00, 0x01, 0x00, 0x00, 0xce, 0xa7};
unsigned char phasePosRes[] = {0x01, 0x10, 0x71, 0x58, 0x00, 0x02, 0xdb, 0x27};


unsigned char bufCyc[] = {0x01, 0x10, 0x71, 0x4a, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char phaseCycRes[] = {0x01, 0x10, 0x71, 0x4a, 0x00, 0x02, 0x7b, 0x22};
unsigned char bufAng[] = {0x01, 0x10, 0x71, 0x4b, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char phaseAngRes[] = {0x01, 0x10, 0x71, 0x4b, 0x00, 0x02, 0x2a, 0xe2};
unsigned char bufSta[] = {0x01, 0x10, 0x71, 0x48, 0x00, 0x02, 0x04, 0x00, 0x5f, 0x00, 0x00, 0xae, 0x79};
unsigned char phaseStaRes[] = {0x01, 0x10, 0x71, 0x48, 0x00, 0x02, 0xda, 0xe2};

unsigned char bufSto[] = {0x01, 0x10, 0x71, 0x48, 0x00, 0x02, 0x04, 0x00, 0x0f, 0x00, 0x00, 0xae, 0x68};
unsigned char phaseStoRes[] = {0x01, 0x10, 0x71, 0x48, 0x00, 0x02, 0xda, 0xe2};


unsigned char bufArr[] = {0x01, 0x20, 0x71, 0x49, 0x00, 0x02, 0x04, 0x04, 0x00, 0x00, 0x00, 0x5e, 0x68};

void dumpBuf(int n, unsigned char *buf, int s)
{
	printf("%d: ", n);
	for(int i = 0; i < s; i++) {
		printf("%x ", buf[i]);
	}
	printf("\n");
}

uint16_t getCRC(unsigned char *cp, unsigned int len)
{
	unsigned int j, i, crc = 0xffff;

	if(len <= 0)
		return 0;

	for(j = 0; j <len; j++) {
		crc = crc^((unsigned int) cp[j]);
		for(i = 0; i < 8; i++) {
			if((crc & 1) != 0) {
				crc = (crc >> 1) ^ 0xA001;
			}
			else {
				crc = crc >> 1;
			}
		}
	}
	return crc;
}

void fillAngBuf(int ang, unsigned char *buf, int len)
{
	uint16_t t;
	unsigned char *c = (unsigned char *)(&t);

	t = (65536 * ang) / 360;
	buf[9] = c[1];
	buf[10] = c[0];

	t = getCRC(buf, len - 2);
	buf[11] = c[0];
	buf[12] = c[1];
}

void fillCycBuf(int cyc, unsigned char *buf, int len)
{
	uint16_t t;
	unsigned char *c = (unsigned char *)(&t);

	if(cyc == -1) {
		buf[7] = 0xff;
		buf[8] = 0xff;
		buf[9] = 0xff;
		buf[10] = 0xff;
	}
	else if(cyc == -2) {
		buf[7] = 0xff;
		buf[8] = 0xfe;
		buf[9] = 0xff;
		buf[10] = 0xff;
	}
	else {
		buf[7] = 0;
		buf[8] = cyc;
		buf[9] = 0;
		buf[10] = 0;
	}

	t = getCRC(buf, len - 2);
	buf[11] = c[0];
	buf[12] = c[1];
}

void fillFin(unsigned char *buf, int len)
{
	uint16_t t;
	unsigned char *c = (unsigned char *)(&t);

	t = getCRC(buf, len - 2);
	buf[11] = c[0];
	buf[12] = c[1];
}

void SerialWorker::buildPhaseCmd(int gui_ang)
{
	int cyc = 0;
	int ang = gui_ang;

	if(ang >= 0) {
		if(ang >= 360) 
			cyc = ang / 360;
		ang = ang % 360;
	}
	else {
		if(ang <= -360) {
			cyc = ang / 360;
		}
		ang = ang % (-360);
		ang = ang + 360;
		cyc--;

		if(ang == 360) {
			ang = 0;
			cyc++;
		}
	}

	// qDebug()<<cyc << ang;
	fillCycBuf(cyc, bufCyc, sizeof(bufCyc) / sizeof(bufCyc[0]));
	fillAngBuf(ang, bufAng, sizeof(bufAng) / sizeof(bufAng[0]));
}

void SerialWorker::doPhaseCmd(int step, int w=0)
{
	int len;
	static int rollPause = 0;

	if(w != 0) {
		rollPause = w;
		// qDebug()<< "step " << step << " wait " << rollPause;
	}
	qDebug()<<"do step " << step;
	switch(step) {	
		case ENABLEPHASE:
			cmd = bufEng;
			len = sizeof(bufEng) / sizeof(bufEng[0]);
			break;

		case SETTYPE:
			cmd = bufPos;
			len = sizeof(bufPos) / sizeof(bufPos[0]);
			break;

		case SETCYC:
			cmd = bufCyc;
			len = sizeof(bufCyc) / sizeof(bufCyc[0]);
			break;

		case SETANG:
			cmd = bufAng;
			len = sizeof(bufAng) / sizeof(bufAng[0]);
			break;

		case STARTR:
			cmd = bufSta;
			len = sizeof(bufSta) / sizeof(bufSta[0]);
			break;

		case STOPR:
			cmd = bufSto;
			len = sizeof(bufSto) / sizeof(bufSto[0]);
			break;

		case CHECKARR:
			//cmd = bufArr;
			//len = sizeof(bufArr) / sizeof(bufArr[0]);
			//break;

		case OVERROLL:
			qDebug()<<"next ang is after " << rollPause;
			QTimer::singleShot(rollPause, this, [=](){
					emit rollfinish();});

		default:
			return;
	}
	//dumpBuf(sec, cmd, len);
	memset(tmpRes, 0, sizeof(tmpRes));
	sfd[COM_PHASE].serial->write((const char *)cmd, len);
	sec = step;
}

SerialWorker::SerialWorker(int f, QObject *parent) : QObject(parent),isFull(f)
{
}

void SerialWorker::getAllDevice(QStringList &listDev)
{
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

	for(int i = 0; i < ports.count(); i++)
		listDev << ports.at(i).portName();

	if(listDev.contains("COM1") || listDev.contains("tnt1"))
		motorReady = 1;
}

void SerialWorker::openDevice(int sid, QString tryDev, int rate)
{
	QSerialPortInfo portInfo;
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

	for (int i = 0; i < ports.count(); i++) {
		if (ports.at(i).portName() == tryDev) {

			portInfo = ports.at(i);
			qDebug()<<"try	"<< sid << " " <<tryDev;
			setSerialPort(sid, &portInfo, rate);
			break;
		}
	}
}

void SerialWorker::run()
{
	if(motorReady == 1)
		openMotor();
}

void SerialWorker::openMotor()
{
#ifdef Q_OS_LINUX
	openDevice(0, "tnt1", MOTOR_SPEED);
#else
	openDevice(0, "COM1", MOTOR_SPEED);
#endif
	doPhaseCmd(0);
}

SerialWorker::~SerialWorker()
{
	closeDevices();
}

void SerialWorker::closeDevices()
{
	for(int sid = 0; sid < 4; sid++)
		closeDevice(sid);
}

void SerialWorker::closeDevice(int sid)
{
	if (sfd[sid].serial != NULL) {
		if (sfd[sid].serial->isOpen()) {
			sfd[sid].serial->clear();
			sfd[sid].serial->close();
		}
		sfd[sid].serial->disconnect();
		delete sfd[sid].serial;
		sfd[sid].serial = NULL;
		qDebug()<<"close " << sid;
	}
}

void SerialWorker::setSerialPort(int sid, QSerialPortInfo *port, int rate )
{
	closeDevice(sid);

	sfd[sid].serial = new QSerialPort(*port);
	sfd[sid].serial->setBaudRate(rate);
	sfd[sid].serial->setDataBits(QSerialPort::Data8);
	sfd[sid].serial->setParity(QSerialPort::NoParity);
	sfd[sid].serial->setStopBits(QSerialPort::OneStop);
	sfd[sid].serial->setFlowControl(QSerialPort::NoFlowControl);
	//sfd[sid].serial->setFlowControl(QSerialPort::HardwareControl);

	if(sfd[sid].serial->isOpen() == false) {
		if (!sfd[sid].serial->open(QIODevice::ReadWrite)) {
			sfd[sid].serial->clear();
			sfd[sid].serial->close();
			delete sfd[sid].serial;
			sfd[sid].serial = NULL;
			return;
		}
	}

	sfd[sid].serial->setDataTerminalReady(true); //you do need to set these or the fan gets dirty
	sfd[sid].serial->setRequestToSend(true);

    connect(sfd[sid].serial, &QSerialPort::readyRead, this, &SerialWorker::readSerialData);
	qDebug()<<"open " << sid << " finish";
}

void SerialWorker::phaseRes()
{
	qDebug()<<"ccccccall " << sec;
	doPhaseCmd(sec);
}

int SerialWorker::checkPhaseRes()
{
	unsigned char *motorRes;
	int resLen;

	switch(sec) {
		case ENABLEPHASE:
			motorRes = phaseEngRes;
			resLen = sizeof(phaseEngRes) / sizeof(phaseEngRes[0]);
			break;

		case(SETTYPE) :
			motorRes = phasePosRes;
			resLen = sizeof(phasePosRes) / sizeof(phasePosRes[0]);
			break;

		case(SETCYC):
			motorRes = phaseCycRes;
			resLen = sizeof(phaseCycRes) / sizeof(phaseCycRes[0]);
			break;

		case(SETANG):
			motorRes = phaseAngRes;
			resLen = sizeof(phaseAngRes) / sizeof(phaseAngRes[0]);
			break;

		case(STARTR):
			motorRes = phaseStaRes;
			resLen = sizeof(phaseStaRes) / sizeof(phaseStaRes[0]);
			break;

		case STOPR:
			motorRes = phaseStoRes;
			resLen = sizeof(phaseStoRes) / sizeof(phaseStoRes[0]);
			break;

		case(CHECKARR):
			return 0;

		default:
			return -1;
	}

	for(int i = 1; i<tmpRes[0]; i++) {
		int j;
		if(tmpRes[0] - i + 1 < resLen)
			return -1;

		for(j = 0; j<resLen; j++) {
			if(tmpRes[i + j] != motorRes[j]) 
				break;
		}
		if(j == resLen)
			return 0;
	}

	return -1;
}

void SerialWorker::dealWithPhaseRes(QByteArray &text)
{	
	for(int i = 0; i < text.size();i++) 
		tmpRes[tmpRes[0]+i+1] = text.at(i);	
	tmpRes[0]+= text.size();

	if(checkPhaseRes() != 0) {
		qDebug()<<"wrong at " << sec << " " << text.toHex();
		return;
	}

#if 1
	qDebug()<<sec <<" phase back " << text.toHex();
	//for(int i= 1; i <= tmpRes[0]; i++)
	//	qDebug()<< QString("%1").arg(tmpRes[i] , 0, 16);
#endif

	if(sec != SETTYPE)
		doPhaseCmd(++sec);
	else
		qDebug()<<"init 2 step finished, waiting";
}

void SerialWorker::dealWithNavRes(int sid, QByteArray &text)
{
	for(int i = 0; i < text.length(); i++) {
		procRXChar(sid, text.at(i));
	}
}

void SerialWorker::procRXChar(int sid, unsigned char c)
{

	if ((c == 0xFF) && (sfd[sid].ffFlag == 0)) { // first 0xFF
		sfd[sid].ffFlag = 1;
		return;
	}

	if (sfd[sid].ffFlag == 0) {
		sfd[sid].res.append(c);
	}
	else {
		sfd[sid].ffFlag = 0;
		if (c == 0xFF) { // 2 0xFF, the first one means escape character
			sfd[sid].res.append(c);
		}
		else if (c == 0x00) { // end of message
			handleFullData(sid);
			sfd[sid].res.clear();
		}
		else { // a new message start
			handleFullData(sid);
			sfd[sid].res.clear();
			sfd[sid].res.append(c);
		}
	}
}

void SerialWorker::procVol(int sid, int pid, QByteArray volS)
{
	unsigned char c[2], v;
	QByteArray res = sfd[sid].res.toHex();

	for(int i = 0; i < 2; i++) {
		c[i] = volS.at(i);
	}
	volS.remove(0,2);

	if(c[0] == 0x4c && c[1] == 0x6f) {
		v = volS.at(3);
		res = res.mid(2, 20);
		if(res[0] == '9')
			res[0]= '1';
		//qDebug()<< "--- " <<res;
		emit updateVol(pid, v, res);
	}
}

void SerialWorker::handleFullData(int sid)
{
	unsigned char c[5];
	QByteArray res = sfd[sid].res;
	int type = -1;

	for(int i = 0; i < 5; i++) {
		c[i] = res.at(i);
	}

	res.remove(0,5);

	if((c[1] & 0x7f) == 0x17 && c[2] == 0x33 && c[3] == 0x31 && c[4] == 0x10) {
		// qDebug()<<id[0] << id[1] << id[2] << id[3];
		//return;
		type = 0;
	}
    if(c[1] == 0x9b && c[2] == 0x00 && c[3] == 0x00 && c[4] == 0x73) {
        type =1;
    }

	if(type == 0) {
		if((c[0] & 0xf8) == 0x50) {//can 1
			//qDebug()<< sid << " can 1 " << res.toHex();

			if(sid == 1)
				procVol(sid, 0, res);
			else if(sid == 2)
				procVol(sid, 2, res);
			else
				qDebug()<<"unknown vol";
		}
		else if ((c[0] & 0xf8) == 0x58) {
			procVol(sid, 1, res);
		}
		else {
			qDebug()<<"unknown";
		}
	}
	else if(type == 1) {
        res.remove(0,6);
        res = res.mid(0,1);
        QString s =res.toHex();

        if((c[0] & 0xf8) == 0x50) {//can 1
            if(sid == 1) {
                if(r1 == "" || r1!= s) {
                    r1 = s;
                    if(r1 != "")
                        informPress(1);//A
                }
            }
            else if(sid == 2) {
                if(r2 == "" || r2 != s) {
                    r2 = s;
                    if(r2 != "")
                        informPress(2);//C
                }
            }
            else {
                qDebug()<<"unknown press";
            }
        }
        else if ((c[0] & 0xf8) == 0x58) {
            if(r3 == "" || r3!= s) {
                r3 = s;
                if(r3 != "")
                    informPress(3);//B
            }
        }
        else {
            qDebug()<<"unknown pres";
        }
	}
}

void SerialWorker::readSerialData()
{
	QByteArray text;
	QObject* obj = sender();
	int sid;

	for(sid = 0; sid < 4; sid++) {
		if(sfd[sid].serial == obj)
			break;
	}

//	qDebug()<<sid;
	if(sid >= 4)
		return;

	text = sfd[sid].serial->readAll();

	if(sid == 0) 
		return dealWithPhaseRes(text);

#if 1
	else
		return dealWithNavRes(sid, text);
#else
	else
		return dealWithNavSerial(sid - 1, text);
#endif
}


void SerialWorker::refind(int sid, QByteArray &text)
{
	static int acc[4] = {0, 0, 0, 0};
	static int last[4] = {0, 0, 0, 0};

	QString prefix = "Rotary_Enc: ";
	if(text.contains("Rotary_Enc: ")) {

		QString Rotary_Enc = text.mid(text.indexOf(prefix));
		QString left = Rotary_Enc.mid(prefix.size());

		int n = left.toInt();
		if(n !=0) {

			int type = n * last[sid];
			if(type < 0)
				acc[sid] = n;
			else
				acc[sid]+=n;
			last[sid] = n;
			emit updateCount(sid, type, n, acc[sid]);
		}
	}
}

void SerialWorker::dealWithNavSerial(int sid, QByteArray &text)
{
	/*static int acc = 0;
	  static int last = 0;*/

	QString prefix = "Rotary_Enc: ";

	if(isFull) {
		emit updateSerialLog(sid, text);
	}
	if(text.contains("Rotary_Enc: ")) {

		/*QString Rotary_Enc = text.mid(text.indexOf(prefix));
		  QString left = Rotary_Enc.mid(prefix.size());

		  int n = left.toInt();
		  if(n !=0) {

		  int type = n * last;
		  if(type < 0)
		  acc = n;
		  else
		  acc+=n;
		  last = n;
		  emit updateCount(type, n, acc);
		  }*/

		refind(sid, text);
		text = text.mid(text.indexOf(prefix) + prefix.size());
	}
}


QByteArray buildCan1_1()
{
    QList <char> cmd;
    QByteArray raw;

    cmd.append(0x50);
    cmd.append(0x05);

    cmd.append(0xbf);
    cmd.append(0x20);
    cmd.append(0x00);
    cmd.append(0x00);
    cmd.append(0x00);
    cmd.append(0xff);
    cmd.append(0x00);

    for(int i = 0; i < cmd.count(); i++)
        raw.append(cmd[i]);
    return raw;
}

QByteArray buildCan2_1()
{
    QList <char> cmd;
    QByteArray raw;

    cmd.append(0x58);
    cmd.append(0x05);

    cmd.append(0xbf);
    cmd.append(0x20);
    cmd.append(0x00);
    cmd.append(0x00);
    cmd.append(0x00);
    cmd.append(0xff);
    cmd.append(0x00);

    for(int i = 0; i < cmd.count(); i++)
        raw.append(cmd[i]);
    return raw;

}

QByteArray buildCan1_2()
{
    QList <char> cmd;
    QByteArray raw;

    cmd.append(0x50);
    cmd.append(0x05);

    cmd.append(0xbf);
    cmd.append(0x00);
    cmd.append(0x00);
    cmd.append(0x00);
    cmd.append(0x00);
    cmd.append(0xff);
    cmd.append(0x00);
    for(int i = 0; i < cmd.count(); i++)
        raw.append(cmd[i]);
    return raw;

}


QByteArray buildCan2_2()
{
    QList <char> cmd;
    QByteArray raw;

    cmd.append(0x58);
    cmd.append(0x05);

    cmd.append(0xbf);
    cmd.append(0x00);
    cmd.append(0x00);
    cmd.append(0x00);
    cmd.append(0x00);
    cmd.append(0xff);
    cmd.append(0x00);
    for(int i = 0; i < cmd.count(); i++)
        raw.append(cmd[i]);
    return raw;

}

void SerialWorker::controlVol()
{
    qDebug()<<"xxx1";

    sfd[1].serial->write(buildCan1_1());//dut1 20
    sfd[1].serial->flush();

    QThread::msleep(100);
    qDebug()<<"xxx2";

    sfd[2].serial->write(buildCan1_1());//dut3 20
    sfd[2].serial->flush();
    QThread::msleep(100);

    qDebug()<<"xxx3";

    sfd[1].serial->write(buildCan1_2()); //dut1 00
    sfd[1].serial->flush();
    QThread::msleep(100);
    qDebug()<<"xxx4";

    sfd[2].serial->write(buildCan1_2()); //dut3 00
    sfd[2].serial->flush();
    QThread::msleep(100);
    qDebug()<<"xxx5";

   sfd[1].serial->write(buildCan2_1()); //dut 2 20
    sfd[1].serial->flush();
    QThread::msleep(100);
    qDebug()<<"xxx6";

    sfd[1].serial->write(buildCan2_2());//dut2 00
    sfd[1].serial->flush();
    QThread::msleep(100);

    qDebug()<<"xxx7";

    /*QByteArray cmd;
    qDebug()<<"xxx 1";

    cmd[0]= 0x50;
    cmd[1]= 0x05;
    cmd[2]= 0xbf;
    cmd[3] = 0x20;
    cmd[4] = 0x00;
    cmd[5] = 0x00;
    cmd[6] = 0x00;
    cmd[7] = 0xff;
    cmd[8] = 0x00;

    sfd[1].serial->write(cmd);//can 1
    QThread::msleep(100);
    qDebug()<<"xxx 2";

    sfd[2].serial->write(cmd);//can 1
    QThread::msleep(100);

    cmd[0]= 0x58;
    sfd[1].serial->write(cmd);//can 2
    qDebug()<<"xxx 3";

    QThread::msleep(100);

    cmd[0]= 0x50;
    cmd[1]= 0x05;
    cmd[2]= 0xbf;
    cmd[3] = 0x00;
    cmd[4] = 0x00;
    cmd[5] = 0x00;
    cmd[6] = 0x00;
    cmd[7] = 0xff;
    cmd[8] = 0x00;

    sfd[1].serial->write(cmd);//can 1
    sfd[2].serial->write(cmd);//can 1

    cmd[0]= 0x58;
    sfd[1].serial->write(cmd);//can 2
    QThread::msleep(500);*/
}

void SerialWorker::openProduct(QStringList d)
{
	for(int i = 0; i < d.size(); i++)
		openDevice(i + 1, d[i], PRODUCT_SPEED);
}
