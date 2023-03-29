#pragma once
#pragma execution_character_set("utf-8")
#include <QWidget>
#include "ui_SettingsDialog.h"
#include <qmessagebox.h>
#include <qfile.h>
#include <qdir.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <QCloseEvent>
#include <qmap.h>
#include <qsettings.h>

#define AUTO_RUN "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"

class SettingsDialog  : public QWidget
{
	Q_OBJECT

public:
	void LoadConnect();
	SettingsDialog(QWidget *parent=nullptr);
	~SettingsDialog();
	bool CheckTime();
	void ReadSettings();
	void closeEvent(QCloseEvent* event);
	QMap<QString,QString> Setting;
	bool Changed=false;
private:
	Ui::Form ui;
signals:
	void SendSettings(QMap<QString,QString>);
private slots:
	void WriteSettings();
	void DeleteReFile();
	void AutoLaunch(bool);
};
