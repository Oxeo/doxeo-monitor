#ifndef SCRIPTCONTROLLER_H
#define SCRIPTCONTROLLER_H

#include "core/abstractcontroller.h"
#include "libraries/scriptengine.h"


class ScriptController : public AbstractController
{
    Q_OBJECT

public:
    ScriptController(QObject *parent = 0);
    void defaultAction();
    void stop();

public slots:
    void scriptList();
    void editor();
    void exportScripts();
    void jsonScriptList();
    void jsonEditScript();
    void jsonDeleteScript();
    void jsonChangeScriptStatus();
    void jsonSetScriptBody();
    void jsonGetScript();

protected:
    ScriptEngine *scriptEngine;
};


#endif // SCRIPTCONTROLLER_H
