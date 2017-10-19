#include "dialogconfig.h"
#include "mainwindow.h"
#include "ui_dialogconfig.h"

#include <QDebug>
DialogConfig::DialogConfig(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogConfig)
{
	mw=(MainWindow *)parent;
	ui->setupUi(this);
	connect(this, &DialogConfig::refresh, mw, &MainWindow::freshSerial);
}

DialogConfig::~DialogConfig()
{
	delete ui;
}

void DialogConfig::setInterface(QStringList &d)
{
	if(d.size() != 3)
		return;

	for(int i = 0; i < d.size(); i++) {

		ui->a->addItem(d[i]);
		ui->b->addItem(d[i]);
		ui->c->addItem(d[i]);
	}

	ui->b->setCurrentIndex(1);
	ui->c->setCurrentIndex(2);
}

void DialogConfig::on_pushButton_clicked()
{
	emit refresh();
}

void DialogConfig::on_buttonBox_accepted()
{
	QStringList s;

	s<<ui->a->currentText();
	s<<ui->b->currentText();
	s<<ui->c->currentText();
	mw->setSerial(s);
	QDialog::accept();
}

void DialogConfig::on_buttonBox_rejected()
{
	QDialog::reject();
}
