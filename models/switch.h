#ifndef SWITCH_H
#define SWITCH_H

#include "core/event.h"
#include "libraries/jeedom.h"
#include "libraries/mysensors.h"

#include <QObject>
#include <QString>
#include <QHash>
#include <QJsonObject>
#include <QDateTime>
#include <QTimer>

class Switch : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ getId)
    Q_PROPERTY(QString status READ getStatus WRITE setStatus)

public:
    explicit Switch(QString id, QObject *parent = 0);

    void setStatus(QString status);
    void updateStatus(QString status);
    QString getId()  const;
    QString getStatus()  const;

    QString getName() const;
    QString getCategory() const;
    QJsonObject toJson() const;
    void setName(const QString &value);
    void setCategory(const QString &value);
    bool flush();
    bool remove();
    void setPowerOnCmd(const QString &value);
    void setPowerOffCmd(const QString &value);
    QString getPowerOnCmd() const;
    QString getPowerOffCmd() const;
    int getOrder() const;
    void setOrder(int value);
    QString getSensorStatus() const;
    QString getSensor() const;
    void setSensor(const QString &value);

    bool getIsVisible() const;
    void setIsVisible(bool value);

    static void update();
    static bool isIdValid(QString id);
    static Switch *get(QString id);
    static QHash<QString, Switch *> &getSwitchList();
    static QList<Switch *> getSortedSwitchList();
    static Event* getEvent();
    static void setJeedom(Jeedom *jeedom);
    static void setMySensors(MySensors *value);
    static void updateStatusByCommand(QString cmd);

public slots:
    void powerOn(int timerOff = 0);
    void powerOnAfter(int timer);
    void powerOff();
    int getLastUpdate(int index) const;

protected slots:
    void updateValue(QString cmd, QString value);

protected:
    static bool compareByOrder(Switch *s1, Switch *s2);

    QString id;
    QString name;
    QString category;
    QString status;
    QString powerOnCmd;
    QString powerOffCmd;
    QString sensor;
    int order;
    bool isVisible;

    QList<QDateTime> lastUpdate;
    QTimer *timerPowerOff;
    QTimer *timerPowerOn;

    static QHash<QString, Switch*> switchList;
    static Event event;
    static Jeedom *jeedom;
    static MySensors *mySensors;
};

#endif // SWITCH_H
