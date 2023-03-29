#pragma once
#pragma execution_character_set("utf-8")
#include <QtWidgets/QWidget>
#include <QtCharts>
using namespace QtCharts;
#include "ui_monitorDWM.h"
#include <QtXlsx/xlsxdocument.h>  //有问题__debugbreak();
//#include <QXlsx/xlsxdocument.h>//也有问题
#include <qmessagebox.h>
#include <qdebug.h>
#include "SettingsDialog.h"
#include <qprocess.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qtimer.h>
#include <QSystemTrayIcon>
#include <qicon.h>
#include <qmenu.h>
#include <qaction.h>
#include <QCloseEvent>
#include <qmap.h>
#include <exception>

class monitorDWM : public QWidget
{
    Q_OBJECT

public:
    QMap<QString,QString> Settings;
    QString ProcessPID;
    QTimer* timer = nullptr;
    QSystemTrayIcon* TrayIcon;
    QMenu* Menu;
    QAction* Open_windows;
    QAction* ExitProgram;
    QString ProSize="0";
    QXlsx::Document* TempXlsx=new QXlsx::Document("temp.xlsx");
    QXlsx::Document* Xlsx = new QXlsx::Document("Record.xlsx");
    int currentNum = 0;

    monitorDWM(QWidget *parent = nullptr);
    ~monitorDWM();
    void loadconnect();
    void GetPid();
    void ReadSetting();
    bool CheckProcess(QString);
    void InitrayIcon();
    void CreateTrayMeun();
    void closeEvent(QCloseEvent* event);
    void DispXlsx();
    void InitCharts();
private:
    Ui::monitorDWMClass ui;
private slots:
    void OpenSettings();
    void ReceiveSettings(QMap<QString, QString>);
    void StartTimer();
    void StopTimer();
    void StartRecord();
    void KillProgram();
    void trayIconActivated(QSystemTrayIcon::ActivationReason);
    void TrayShow_windows();
    void TrayClose_windows();
};
