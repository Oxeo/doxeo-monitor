#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include "controllers/cameracontroller.h"
#include <libraries/gsm.h>
#include <libraries/jeedom.h>
#include <libraries/mysensors.h>
#include <libraries/thermostat.h>
#include <models/script.h>

#include <QObject>
#include <QJSEngine>
#include <QTimer>
#include <QString>

class ScriptEngine : public QObject
{
    Q_OBJECT

public:
    explicit ScriptEngine(Thermostat *thermostat,
                          Jeedom *jeedom,
                          Gsm *gsm,
                          MySensors *mySensors,
                          CameraController *cameraController,
                          QObject *parent = 0);
    void init();
    QString runCmd(QString cmd);

public slots:
    void run(QString event = "scheduler");

protected slots:
    void updateSensors();
    void updateSwitches();
    void updateHeaters();
    void switchValueUpdated(QString id, QString type, QString value);
    void sensorValueUpdated(QString id, QString type, QString value);
    void heaterValueUpdated(QString id, QString type, QString value);
    void newSMS(QString numbers, QString msg);
    void settingValueUpdated(QString id, QString type, QString value);
    void newMessageFromMessageLogger(QString type, QString message);
    void eventTimeout(QString name);
    void cameraStreamRequested(int id);

protected:
    QString getLibraries();

    QJSEngine engine;
    QTimer *timer;
    QHash<QString, uint> eventList;
    Gsm *gsm;
    Jeedom *jeedom;
    MySensors *mySensors;
    Thermostat *thermostat;
    CameraController *cameraController;
};

#endif // SCRIPTENGINE_H
