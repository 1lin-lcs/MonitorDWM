#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	LoadConnect();
	ReadSettings();
}

SettingsDialog::~SettingsDialog()
{}

void SettingsDialog::LoadConnect() {
	connect(ui.SaveButton, SIGNAL(clicked()), this, SLOT(WriteSettings()));
	connect(ui.radioButton, SIGNAL(toggled(bool)), this, SLOT(AutoLaunch(bool)));
}


bool SettingsDialog::CheckTime() {
	int Hour = ui.HourSpinBox->value();
	int Minute = ui.MinuteSpinBox->value();
	int Second = ui.SecondSpinBox->value();
	if (Hour == 0 && Minute == 0 && Second == 0) {
		QMessageBox::information(this, "错误", "时间不能全为0");
		return false;
	}
	return true;
}

void SettingsDialog::ReadSettings() {
	if (!QFile::exists(QDir::currentPath() + "/Settings.json"))
	{
		ui.MonitorproEdit->setText("");
		Setting.insert("Program", "");
		ui.HourSpinBox->setValue(0);
		Setting.insert("Hour", "0");
		ui.MinuteSpinBox->setValue(0);
		Setting.insert("Minute", "0");
		ui.SecondSpinBox->setValue(1);
		Setting.insert("Second", "1");
		ui.radioButton->setChecked(false);
		Setting.insert("Launch_with_Windows", "false");
		ui.LimitLineEdit->setText("1");
		Setting.insert("LimitSize", "1");
		ui.DNumLineEdit->setText("");
		Setting.insert("DisplayNum", "");
		return;
	}
	Setting.clear();
	Changed = false;
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
		ui.MonitorproEdit->setText(Program.toString());
		Setting.insert("Program", ui.MonitorproEdit->text());
	}
	if (JsonObject.contains("Time")) {
		QJsonValue Time = JsonObject.value("Time");
		QJsonArray TimeArray = Time.toArray();
		ui.HourSpinBox->setValue(TimeArray.at(0).toInt());
		Setting.insert("Hour", QString::number(ui.HourSpinBox->value()));
		ui.MinuteSpinBox->setValue(TimeArray.at(1).toInt());
		Setting.insert("Minute", QString::number(ui.MinuteSpinBox->value()));
		ui.SecondSpinBox->setValue(TimeArray.at(2).toInt());
		Setting.insert("Second", QString::number(ui.SecondSpinBox->value()));
	}
	if (JsonObject.contains("Launch_with_Windows")) {
		QJsonValue Launch = JsonObject.value("Launch_with_Windows");
		ui.radioButton->setChecked(Launch.toBool());
		Setting.insert("Launch_with_Windows", QVariant(ui.radioButton->isChecked()).toString());
	}
	if (JsonObject.contains("LimitSize"))
	{
		QJsonValue LimitSize = JsonObject.value("LimitSize");
		ui.LimitLineEdit->setText(LimitSize.toString());
		Setting.insert("LimitSize", ui.LimitLineEdit->text());
	}if (JsonObject.contains("DisplayNum"))
	{
		QJsonValue LimitSize = JsonObject.value("DisplayNum");
		ui.DNumLineEdit->setText(LimitSize.toString());
		Setting.insert("DisplayNum", ui.DNumLineEdit->text());
	}
	file.close();
}

void SettingsDialog::WriteSettings() {
	QFile file(QDir::currentPath()+"/Settings.json");
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::information(this, "错误", "无法写入设置文件");
		return;
	}
	QJsonObject JsonObject;
	QJsonArray TimeArray;
	if (!CheckTime()) return;
	TimeArray.append(ui.HourSpinBox->value());
	TimeArray.append(ui.MinuteSpinBox->value());
	TimeArray.append(ui.SecondSpinBox->value());
	JsonObject.insert("Program", ui.MonitorproEdit->text());
	JsonObject.insert("Time", TimeArray);
	JsonObject.insert("Launch_with_Windows", ui.radioButton->isChecked());
	JsonObject.insert("LimitSize", ui.LimitLineEdit->text());
	JsonObject.insert("DisplayNum", ui.DNumLineEdit->text());
	QJsonDocument JDoc(JsonObject);
	QByteArray Json(JDoc.toJson());
	file.write(Json);
	file.close();
	QMessageBox::information(this, "提示", "保存成功");
	Changed = true;
	this->close();
}

void SettingsDialog::closeEvent(QCloseEvent* event) {
	if (Changed) {
		event->accept();
		return;
	}
	bool Is_Changed=false;
	QStringList changed;
	QString temp;
	temp = Setting.find("Program").value();
	if (QString::compare(temp, ui.MonitorproEdit->text()) !=0 ) {
		changed.append("true");
	}
	temp = Setting.find("Hour").value();
	int Ntemp = temp.toInt();
	if (Ntemp != ui.HourSpinBox->value()) {
		changed.append("true");
	}
	temp = Setting.find("Minute").value();
	Ntemp = temp.toInt();
	if (Ntemp != ui.MinuteSpinBox->value()) {
		changed.append("true");
	}
	temp = Setting.find("Second").value();
	Ntemp = temp.toInt();
	if (Ntemp != ui.SecondSpinBox->value()) {
		changed.append("true");
	}
	temp = Setting.find("Launch_with_Windows").value();
	bool Btemp = ui.radioButton->isChecked();
	if (QString::compare(temp, QVariant(Btemp).toString())!=0) {
		changed.append("true");
	}
	temp = Setting.find("LimitSize").value();
	if (QString::compare(temp, ui.LimitLineEdit->text()))
	{
		changed.append("true");
	}
	for (int i = 0; i < changed.size(); i++) {
		if (QString::compare(changed.at(i), "true") == 0)
			Is_Changed = true;
	}

	if (Is_Changed) {
		auto temp = QMessageBox::information(this, "提示", "未保存，是否退出？", QMessageBox::Yes | QMessageBox::No);
		if (temp == QMessageBox::Yes)
		{
			emit SendSettings(Setting);
			event->accept();
		}
		else
		{
			event->ignore();
		}
	}
	else
	{
		emit SendSettings(Setting);
		event->accept();
	}
}

void SettingsDialog::DeleteReFile() {
	if (QFile::exists(QDir::currentPath() + "/Record.xlsx")) {
		QFile::remove(QDir::currentPath() + "/Record.xlsx");
		QMessageBox::information(this, "提示", "已删除");
	}
}

void SettingsDialog::AutoLaunch(bool boost) {
	QSettings* auto_run = new QSettings(AUTO_RUN, QSettings::NativeFormat);
	QString path = QCoreApplication::applicationFilePath();//"E:/文件/Documents/QT Learn/monitorDWM/x64/Debug/monitorDWM.exe"
	if (boost) {
		auto_run->setValue("monitorDWM.exe",path+" min");
		//qDebug() << path + " min";
	}
	else {
		auto_run->remove("monitorDWM.exe");
	}
}