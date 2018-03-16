#include "switch.h"
#include "core/database.h"
#include "libraries/device.h"
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>
#include <QSqlError>
#include <QProcess>

QHash<QString, Switch*> Switch::switchList;
Event Switch::event;

Switch::Switch(QString id, QObject *parent) : QObject(parent)
{
    this->id = id;
    this->order = 0;

    for (int i=0; i<5; i++) {
        lastUpdate.append(QDateTime::currentDateTime().addYears(-1));
    }

    timerPowerOff.setSingleShot(true);
    connect(&timerPowerOff, SIGNAL(timeout()), this, SLOT(powerOff()), Qt::QueuedConnection);
    connect(Device::Instance(), SIGNAL(dataReceived(QString, QString)), this, SLOT(updateValue(QString, QString)), Qt::QueuedConnection);
}

void Switch::setStatus(QString status)
{
    if (status == this->status) {
        return;
    }

    this->status = status;
    this->lastUpdate.prepend(QDateTime::currentDateTime());
    this->lastUpdate.removeLast();

    // Update database
    QSqlQuery query = Database::getQuery();
    query.prepare("UPDATE switch SET status=? WHERE id=?");
    query.addBindValue(status);
    query.addBindValue(this->id);

    Database::exec(query);
    Database::release();

    qDebug() << "Switch status " + id + " set to " + status;

    emit Switch::event.valueChanged(this->id, status);
}

QString Switch::getId() const
{
    return id;
}

QString Switch::getStatus() const
{
    return status;
}

void Switch::powerOn(int timerOff)
{
    if (timerOff > 0) {
        timerPowerOff.start(timerOff*1000);
    } else {
        timerPowerOff.stop();
    }

    if (powerOnCmd.trimmed() != "") {
        Device::Instance()->send(powerOnCmd.split(",").value(0));
    }
    setStatus("on");
}

void Switch::powerOff()
{
    if (powerOffCmd.trimmed() != "") {
        Device::Instance()->send(powerOffCmd.split(",").value(0));
    }
    setStatus("off");
    timerPowerOff.stop();
}

void Switch::update()
{
    QSqlQuery query = Database::getQuery();
    query.prepare("SELECT id, status, name, category, order_by, power_on_cmd, power_off_cmd FROM switch");

    if(Database::exec(query))
    {
        switchList.clear();
        while(query.next())
        {
            Switch *sw = new Switch(query.value(0).toString());

            sw->status = query.value(1).toString();
            sw->name = query.value(2).toString();
            sw->category = query.value(3).toString();
            sw->order = query.value(4).toInt();
            sw->powerOnCmd = query.value(5).toString();
            sw->powerOffCmd = query.value(6).toString();

            switchList.insert(sw->id, sw);
        }
    }

    Database::release();
    emit Switch::event.dataChanged();
}

bool Switch::isIdValid(QString id)
{
    return switchList.contains(id);
}

Switch *Switch::get(QString id)
{
    return switchList[id];
}

void Switch::setName(const QString &value)
{
    name = value;
}

void Switch::setCategory(const QString &value)
{
    category = value;
}

QString Switch::getName() const
{
    return name;
}

QString Switch::getCategory() const
{
    return category;
}

QJsonObject Switch::toJson() const
{
    QJsonObject result;

    result.insert("id", id);
    result.insert("name", name);
    result.insert("category", category);
    result.insert("order", order);
    result.insert("power_on_cmd", powerOnCmd);
    result.insert("power_off_cmd", powerOffCmd);
    result.insert("status", status);

    return result;
}

QHash<QString, Switch *> &Switch::getSwitchList()
{
    return switchList;
}

Event *Switch::getEvent()
{
    return &event;
}

QString Switch::getPowerOnCmd() const
{
    return powerOnCmd;
}

QString Switch::getPowerOffCmd() const
{
    return powerOffCmd;
}

void Switch::setPowerOnCmd(const QString &value)
{
    powerOnCmd = value;
}

void Switch::setPowerOffCmd(const QString &value)
{
    powerOffCmd = value;
}

bool Switch::flush(bool newObject)
{
    QSqlQuery query = Database::getQuery();

    if (!newObject) {
        query.prepare("UPDATE switch SET name=?, category=?, order_by=?, power_on_cmd=?, power_off_cmd=?, status=? WHERE id=?");
    } else {
        query.prepare("INSERT INTO switch (name, category, order_by, power_on_cmd, power_off_cmd, status, id) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?)");
    }
    query.addBindValue(name);
    query.addBindValue(category);
    query.addBindValue(order);
    query.addBindValue(powerOnCmd);
    query.addBindValue(powerOffCmd);
    query.addBindValue(status);
    query.addBindValue(id);

    if (Database::exec(query)) {
        Database::release();
        return true;
    } else {
        Database::release();
        return false;
    }
}

bool Switch::remove()
{
    QSqlQuery query = Database::getQuery();

    query.prepare("DELETE FROM switch WHERE id=?");
    query.addBindValue(id);

    if (Database::exec(query)) {
        Database::release();
        return true;
    } else {
        Database::release();
        return false;
    }
}

void Switch::updateValue(QString id, QString value)
{
    if (value != "" && getPowerOnCmd().split(",").contains(id + ";" + value, Qt::CaseInsensitive)) {
        setStatus("on");
    } else if (value != "" && getPowerOffCmd().split(",").contains(id + ";" + value, Qt::CaseInsensitive)) {
        setStatus("off");
    }
}
int Switch::getOrder() const
{
    return order;
}

void Switch::setOrder(int value)
{
    order = value;
}


int Switch::getLastUpdate(int index) const
{
    return (QDateTime::currentDateTime().toTime_t() - lastUpdate.at(index).toTime_t()) / 60;
}

