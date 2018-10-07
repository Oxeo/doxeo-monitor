#ifndef SCENARIOCONTROLLER_H
#define SCENARIOCONTROLLER_H

#include "core/abstractcontroller.h"
#include "libraries/scriptengine.h"
#include "libraries/gsm.h"

#include <QList>
#include <QString>

class ScenarioController : public AbstractController
{
    Q_OBJECT

public:
    ScenarioController(ScriptEngine *scriptEngine, QObject *parent = 0);
    void defaultAction();
    void stop();

public slots:
    void scenarioList();
    void jsonScenarioList();
    void jsonEditScenario();
    void jsonDeleteScenario();
    void jsonGetScenario();
    void jsonStartScenario();

protected:
    ScriptEngine *scriptEngine;
};


#endif // SCENARIOCONTROLLER_H