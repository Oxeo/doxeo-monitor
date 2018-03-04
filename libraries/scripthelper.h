#ifndef SCRIPTHELPER_H
#define SCRIPTHELPER_H

#include <QObject>
#include <QString>
#include <QHash>

class ScriptHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int day READ getDay)
    Q_PROPERTY(int dayOfWeek READ getDayOfWeek)
    Q_PROPERTY(int hour READ getHour)
    Q_PROPERTY(int minute READ getMinute)

public:
    explicit ScriptHelper(QObject *parent = 0);
    int getDay();
    int getDayOfWeek();
    int getHour();
    int getMinute();

signals:

public slots:
    void sendCmd(QString cmd);
    QString execute(QString cmd);
    void setLog(QString log);
    void setWarning(QString warning);
    void setAlert(QString alert);
    void sendFCM(QString type, QString name, QString body);

    static QString getData(QString key);
    static void setData(QString key, QString value);

protected:
    static QHash<QString, QString> data;
};

#endif // SCRIPTHELPER_H
