#include "monitorDWM.h"

monitorDWM::monitorDWM(QWidget *parent)
    : QWidget(parent)
{  
    ui.setupUi(this);
    loadconnect();
	ReadSetting();
	InitrayIcon();
	InitCharts();
}

monitorDWM::~monitorDWM()
{

	if (timer!=nullptr&&timer->isActive())
	{
		timer->stop();
	}
	DispXlsx();
	if (QFile::exists(QDir::currentPath() + "/temp.xlsx")) {
		QFile::remove(QDir::currentPath() + "/temp.xlsx");
	}
}

void monitorDWM::loadconnect()
{
    connect(ui.SettingsButton, SIGNAL(clicked()),this,SLOT(OpenSettings()));
	connect(ui.StartButton, SIGNAL(clicked()), this, SLOT(StartTimer()));
	connect(ui.StopButton, SIGNAL(clicked()), this, SLOT(StopTimer()));
	connect(ui.StopProButton, SIGNAL(clicked()), this, SLOT(KillProgram()));
	connect(ui.QuitButton, SIGNAL(clicked()), this, SLOT(TrayClose_windows()));
}

void monitorDWM::OpenSettings()
{
    SettingsDialog* dialog = new SettingsDialog();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, SIGNAL(SendSettings(QMap<QString, QString>)), this, SLOT(ReceiveSettings(QMap<QString, QString>)));
    dialog->show();
}

void monitorDWM::ReceiveSettings(QMap<QString,QString> settings) {
	Settings.clear();
	Settings = settings;
	if(!CheckProcess(Settings.find("Program").value()))return;
	qint64 second = Settings.find("Second").value().toInt();
	qint64 minute = Settings.find("Minute").value().toInt();
	qint64 hour = Settings.find("Hour").value().toInt();
	qint64 time = (hour * 3600 + minute * 60 + second) * 1000;
	if (timer!=nullptr&&timer->isActive())
	{
		timer->stop();
		timer->start(time);
	}
}

bool monitorDWM::CheckProcess(QString Process)
{
	QProcess process;
	process.start("tasklist");
	process.waitForFinished();
	QByteArray result = process.readAllStandardOutput();
	process.close();
	QString output = result;
	if (!output.contains(Process)) {
		QMessageBox::information(this, "错误", "进程不存在");
		return false;
	}
	return true;
}

void monitorDWM::ReadSetting() {
	if (!QFile::exists(QDir::currentPath() + "/Settings.json"))
	{
		return;
	}
	QFile file(QDir::currentPath() + "/Settings.json");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::information(this, "错误", "无法读取设置文件");
		return;
	}
	QString Data = file.readAll();
	QJsonParseError ParseError;
	QJsonDocument JDoc = QJsonDocument::fromJson(Data.toUtf8(), &ParseError);
	if (!(ParseError.error == QJsonParseError::NoError)) {
		QMessageBox::information(this, "错误", "无法读取设置文件");
		return;
	}
	QJsonObject JsonObject = JDoc.object();
	if (JsonObject.contains("Program")) {
		QJsonValue Program = JsonObject.value("Program");
		Settings.insert("Program", Program.toString());
	}
	if (JsonObject.contains("Time")) {
		QJsonValue Time = JsonObject.value("Time");
		QJsonArray TimeArray = Time.toArray();
		Settings.insert("Hour", QString::number(TimeArray.at(0).toInt()));
		Settings.insert("Minute", QString::number(TimeArray.at(1).toInt()));
		Settings.insert("Second", QString::number(TimeArray.at(2).toInt()));
	}
	if (JsonObject.contains("Launch_with_Windows")) {
		QJsonValue Launch = JsonObject.value("Launch_with_Windows");
		Settings.insert("Launch_with_Windows", QVariant(Launch.toBool()).toString());
	}
	if (JsonObject.contains("LimitSize")) {
		QJsonValue LimitSize = JsonObject.value("LimitSize");
		Settings.insert("LimitSize",LimitSize.toString());
	}
	if (JsonObject.contains("DisplayNum")) {
		QJsonValue DisplayNum = JsonObject.value("DisplayNum");
		Settings.insert("DisplayNum", DisplayNum.toString());
	}
	file.close();
}

void monitorDWM::StartTimer() {
	ui.StopButton->setEnabled(true);
	ui.StopProButton->setEnabled(true);
	ui.StartButton->setEnabled(false);
	ui.QuitButton->setEnabled(false);
	if(!CheckProcess(Settings.find("Program").value()))return;
	timer = new QTimer(this);
	qint64 second = Settings.find("Second").value().toInt();
	qint64 minute = Settings.find("Minute").value().toInt();
	qint64 hour = Settings.find("Hour").value().toInt();
	qint64 time = (hour * 3600 + minute * 60 + second) * 1000;
	timer->start(time);
	GetPid();
	StartRecord();
	connect(timer,SIGNAL(timeout()),this,SLOT(StartRecord()));
	ui.PNameLabel->setText(Settings.find("Program").value());
	ui.StateLabel->setText("开");
	QMessageBox::information(this, "提示", "已开始监视");
}

void monitorDWM::StopTimer() {
	timer->stop();
	ui.StartButton->setEnabled(true);
	ui.StopButton->setEnabled(false);
	ui.StopProButton->setEnabled(false);
	ui.QuitButton->setEnabled(true);
	ui.StateLabel->setText("关");
	QMessageBox::information(this, "提示", "已停止监视");
	TempXlsx->save();
}

void monitorDWM::StartRecord() {
	QProcess process;
	process.start("tasklist /FI \"PID EQ "+QString::number(ProcessPID.toInt())+" \"");
	process.waitForFinished();
	QByteArray result = process.readAllStandardOutput();
	process.close();
	QString output = result;
	QStringList data = output.split("\r\n");
	QString pro = data.at(3);
	QStringList proc = pro.split(" ", QString::SkipEmptyParts);
	QStringList temp = proc.at(4).split(",");
	QString tem = temp.at(0) + temp.at(1);
	int size = tem.toInt();
	//qDebug() << proc.at(4)<<QString::number(size);
	ProSize = QString::number(size / 1024) + "MB";
	TrayIcon->setToolTip("当前监视程序占用" + ProSize);
	ui.NowLabel->setText(ProSize);
	if (ProSize.split("MB")[0].toInt() >= Settings.find("LimitSize").value().toInt() && !(this->isVisible()))
	{
		TrayIcon->showMessage("提醒", "应用占内存超过设定值！当前值"+ProSize);
	}
	currentNum++;
	QDateTime date = QDateTime::currentDateTime();
	TempXlsx->write("A" + QString::number(currentNum), date.toString("yyyy-MM-dd hh:mm:ss"));
	TempXlsx->write("B" + QString::number(currentNum), ProSize);
}

void monitorDWM::GetPid() {
	QProcess process;
	process.start("tasklist");
	process.waitForFinished();
	QByteArray result = process.readAllStandardOutput();
	process.close();
	QString output = result;
	QStringList data = output.split("\r\n");
	for (int i = 3; i < data.size(); i++)
	{
		if (data.at(i).contains(Settings.find("Program").value()))
		{
			QStringList p = data.at(i).split(" ", QString::SkipEmptyParts);
			//qDebug() << p[1];
			ProcessPID = p[1];
			ui.PPIDLabel->setText(ProcessPID);
			return;
		}
	}
	QMessageBox::information(this, "错误", "未能找到程序PID");
}

void monitorDWM::KillProgram() {
	auto temp = QMessageBox::information(this, "提示", "是否结束程序？", QMessageBox::Yes | QMessageBox::No);
	if (temp == QMessageBox::Yes)
	{
		QProcess process;
		timer->stop();
		QMessageBox::information(this, "提示", "停止监视");
		process.start("taskkill /f /t /pid " + ProcessPID);
		process.waitForFinished();
		process.close();
		QMessageBox::information(this, "提示", "已经结束进程" + Settings.find("Program").value());
	}
	else{
		return;
	}
}

void monitorDWM::InitrayIcon() {
	if (!QFile::exists(QDir::currentPath() + "/tuopan.png")) {
		QMessageBox::information(this, "错误", "托盘图标缺失");
		TrayClose_windows();
		return;
	}
	TrayIcon = new QSystemTrayIcon(this);
	QIcon Icon = QIcon(QDir::currentPath() + "/tuopan.png");
	TrayIcon->setIcon(Icon);
	TrayIcon->setToolTip("当前未监视程序");
	connect(TrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
	CreateTrayMeun();
	TrayIcon->show();
}

void monitorDWM::CreateTrayMeun() {
	Menu = new QMenu(this);
	Open_windows = new QAction("打开主界面", this);
	connect(Open_windows, SIGNAL(triggered()), this, SLOT(TrayShow_windows()));
	ExitProgram = new QAction("退出", this);
	connect(ExitProgram, SIGNAL(triggered()), this, SLOT(TrayClose_windows()));
	Menu->addAction(Open_windows);
	Menu->addAction(ExitProgram);
}

void monitorDWM::TrayShow_windows() {
	if (!this->isVisible()) {
		this->show();
	}
}

void monitorDWM::TrayClose_windows() {
	this->hide();
	this->close();
}

void monitorDWM::closeEvent(QCloseEvent* event) {
	if (this->isVisible()) {
		event->ignore();
		this->hide();
	}
	else {
		event->accept();
	}
}

void monitorDWM::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
	switch (reason)
	{
	case QSystemTrayIcon::Unknown:
		break;
	case QSystemTrayIcon::Context:
		break;
	case QSystemTrayIcon::DoubleClick:
		this->show();
		break;
	case QSystemTrayIcon::Trigger:
		TrayIcon->setContextMenu(Menu);
		break;
	case QSystemTrayIcon::MiddleClick:
		break;
	default:
		break;
	}
}

void monitorDWM::DispXlsx() {
	if (!QFile::exists(QDir::currentPath() + "/temp.xlsx"))
		return;
	QXlsx::Document* tem = new QXlsx::Document("temp.xlsx");
	int rowcount=0;
	int temrowcount = 0;
	try {
		rowcount = Xlsx->dimension().lastRow();
		temrowcount = tem->dimension().lastRow();
	}
	catch (QString s)
	{
		return;
	}
	if (rowcount < 0) {
		rowcount = 0;
	}
	//qDebug() << rowcount << " " <<temrowcount;
	try {
		for (int i = 1; i < temrowcount; i++)
		{
			Xlsx->write("A" + QString::number(rowcount + i), tem->cellAt(i, 1)->value().toString().trimmed());
			Xlsx->write("B" + QString::number(rowcount + i), tem->cellAt(i, 2)->value().toString().trimmed());
		}
	}
	catch (QString s)
	{
		QMessageBox::information(this, "警告", "数据可能损坏");
	}
	Xlsx->save();
}

void monitorDWM::InitCharts() {
	if (!QFile::exists(QDir::currentPath() + "/Record.xlsx")) {
		return;
	}
	QtCharts::QChart* chart = new QtCharts::QChart();
	QXlsx::Document* doc = new QXlsx::Document("Record.xlsx");
	int rowcount = 0,ChartBegin=0;
	rowcount = doc->dimension().lastRow();
	QStringList date,data;
	for (int i = 1; i < rowcount; i++)
	{
		date.append(doc->cellAt(i, 1)->value().toString().trimmed());
		data.append(doc->cellAt(i, 2)->value().toString().trimmed().split("M")[0]);
	}
	if (Settings.count("DisplayNum")==0)
	{
		return;
	}
	if (Settings.find("DisplayNum").value()!="" && rowcount >= Settings.find("DisplayNum").value().toInt()) {
		ChartBegin = data.size() - Settings.find("DisplayNum").value().toInt();
	}
	QLineSeries* series = new QLineSeries;
	QDateTimeAxis* timeAxis = new QDateTimeAxis;
	QValueAxis* yAxis = new QValueAxis;
	timeAxis->setRange(QDateTime::fromString(date.at(0), "yyyy-MM-dd hh:mm:ss"), QDateTime::fromString(data.at(data.size() - 1), "yyyy-MM-dd hh:mm:ss"));
	timeAxis->setMin(QDateTime::fromString(date.at(0), "yyyy-MM-dd hh:mm:ss"));
	timeAxis->setMax(QDateTime::fromString(data.at(data.size() - 1), "yyyy-MM-dd hh:mm:ss"));
	//qDebug() << QDateTime::fromString(date.at(0), "yyyy-MM-dd hh:mm:ss")<< QDateTime::fromString(date.at(date.size() - 1), "yyyy-MM-dd hh:mm:ss");
	timeAxis->setFormat("MM-dd hh:mm");
	timeAxis->setTickCount(date.size());
	timeAxis->setLabelsAngle(60);
	yAxis->setRange(50, 500);
	yAxis->setTickCount(20);
	//series->attachAxis(timeAxis);
	//series->attachAxis(yAxis);
	chart->setTitle("最近占用情况");
	chart->legend()->hide();
	for (int i = ChartBegin; i < date.size(); i++) {
		series->append(QDateTime::fromString(date.at(i), "yyyy-MM-dd hh:mm:ss").toMSecsSinceEpoch(), data.at(i).toInt());
		//qDebug() << QDateTime::fromString(date.at(i), "yyyy-MM-dd hh:mm:ss").toMSecsSinceEpoch();
	}
	chart->addSeries(series);
	chart->setAxisX(timeAxis, series);//注意顺序
	chart->setAxisY(yAxis, series);
	ui.widget->setChart(chart);
	//qDebug() << "完成";
}