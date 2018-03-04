#include "scripthelper.h"
#include "device.h"
#include "firebasecloudmessaging.h"
#include "messagelogger.h"

#include <QDate>
#include <QTime>
#include <QProcess>
#include <QDebug>
#include <QDir>

QHash<QString, QString> ScriptHelper::data;

ScriptHelper::ScriptHelper(QObject *parent) : QObject(parent)
{

}

int ScriptHelper::getDay()
{
    return QDate::currentDate().day();
}

int ScriptHelper::getDayOfWeek()
{
    return QDate::currentDate().dayOfWeek();
}

int ScriptHelper::getHour()
{
    return QTime::currentTime().hour();
}

int ScriptHelper::getMinute()
{
    return QTime::currentTime().minute();
}

void ScriptHelper::sendCmd(QString cmd)
{
    Device::Instance()->send(cmd);
}

QString ScriptHelper::execute(QString cmd)
{
    QProcess process;

    process.setWorkingDirectory(QDir::currentPath());
    process.start(cmd);
    process.waitForFinished(2000);

    if (process.exitCode() != 0) {
        qCritical() << "Unable to execute cmd: " << process.readAll() << process.readAllStandardError();
        return process.readAll() + process.readAllStandardError();
    } else {
        return process.readAll();
    }
}

void ScriptHelper::setLog(QString log)
{
    MessageLogger::logger().addMessage("debug", log);
}

void ScriptHelper::setWarning(QString warning)
{
    MessageLogger::logger().addMessage("warning", warning);
}

void ScriptHelper::setAlert(QString alert)
{
    MessageLogger::logger().addMessage("critical", alert);
}

QString ScriptHelper::getData(QString key)
{
    return data.value(key, "");
}

void ScriptHelper::setData(QString key, QString value)
{
    data.insert(key, value);
}

void ScriptHelper::sendFCM(QString type, QString name, QString body)
{
    FirebaseCloudMessaging *fcm = MessageLogger::logger().getFCM();
    FirebaseCloudMessaging::Message msg = {type.toUpper(), name, body};
    fcm->send(msg);
}
