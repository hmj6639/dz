#ifndef DIALOGCONFIG_H
#define DIALOGCONFIG_H

#include <QDialog>

namespace Ui {
class DialogConfig;
}

class MainWindow;
class DialogConfig : public QDialog
{
	Q_OBJECT

public:
	explicit DialogConfig(QWidget *parent = 0);
	~DialogConfig();
	void setInterface(QStringList &);

private slots:
	void on_pushButton_clicked();

	void on_buttonBox_accepted();

	void on_buttonBox_rejected();

private:
	Ui::DialogConfig *ui;
	MainWindow *mw;

signals:
	void refresh();
};

#endif // DIALOGCONFIG_H
