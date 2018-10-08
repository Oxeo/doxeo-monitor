#include "scenariocontroller.h"
#include "libraries/authentification.h"
#include "models/scenario.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

ScenarioController::ScenarioController(ScriptEngine *scriptEngine, QObject *parent) : AbstractController(parent)
{
    router.insert("list", "scenarioList");
    router.insert("editor", "editor");
    router.insert("scenario_list.js", "jsonScenarioList");
    router.insert("create_scenario.js", "jsonCreateScenario");
    router.insert("edit_scenario.js", "jsonEditScenario");
    router.insert("delete_scenario.js", "jsonDeleteScenario");
    router.insert("get_scenario.js", "jsonGetScenario");
    router.insert("change_scenario_status.js", "jsonChangeStatus");

    this->scriptEngine = scriptEngine;
    Scenario::update();
}

void ScenarioController::defaultAction()
{

}

void ScenarioController::stop()
{
}

void ScenarioController::scenarioList()
{
    if (!Authentification::auth().isConnected(header, cookie)) {
        redirect("/auth");
        return;
    }

    QHash<QString, QByteArray> view;
    view["head"] = loadHtmlView("views/scenario/scenariolist.head.html", NULL, false);
    view["content"] = loadHtmlView("views/scenario/scenariolist.body.html", NULL, false);
    view["bottom"] = loadHtmlView("views/scenario/scenariolist.js", NULL, false);
    loadHtmlView("views/template.html", &view);
}

void ScenarioController::jsonScenarioList()
{
    QJsonObject result;

    if (!Authentification::auth().isConnected(header, cookie)) {
        result.insert("Result", "ERROR");
        result.insert("Message", "You are not logged.");
        loadJsonView(result);
        return;
    }

    QList<Scenario> list = Scenario::getScenarioList().values();
    QJsonArray array;

    foreach (const Scenario &s, list) {
        array.push_back(s.toJson());
    }

    result.insert("Result", "OK");
    result.insert("Records", array);

    loadJsonView(result);
}

void ScenarioController::jsonCreateScenario()
{
    editCreate(true);
}

void ScenarioController::jsonEditScenario()
{
    editCreate(false);
}

void ScenarioController::editCreate(bool newObject)
{
    QJsonObject result;

    if (!Authentification::auth().isConnected(header, cookie)) {
        result.insert("Result", "ERROR");
        result.insert("Message", "You are not logged.");
        loadJsonView(result);
        return;
    }

    Scenario s(query->getItem("id"));

    s.setName(query->getItem("name"));
    s.setDescription(query->getItem("description"));
    s.setContent(query->getItem("content"));
    s.setStatus(query->getItem("status"));
    s.setOrder(query->getItem("order").toInt());
    s.setHide(query->getItem("hide") == "true" ? true : false);
    s.flush(newObject);

    Scenario::update();
    result.insert("Result", "OK");
    result.insert("Record", s.toJson());

    loadJsonView(result);
}

void ScenarioController::jsonDeleteScenario()
{
    QJsonObject result;

    if (!Authentification::auth().isConnected(header, cookie)) {
        result.insert("Result", "ERROR");
        result.insert("Message", "You are not logged.");
        loadJsonView(result);
        return;
    }

    Scenario s(query->getItem("id"));

    if (s.remove()) {
        Scenario::update();
        result.insert("Result", "OK");
    } else {
        result.insert("Result", "ERROR");
    }

    loadJsonView(result);
}

void ScenarioController::jsonGetScenario()
{
    QJsonObject result;

    if (!Authentification::auth().isConnected(header, cookie)) {
        result.insert("msg", "You are not logged.");
        result.insert("success", false);
    }
    else if (Scenario::isIdValid(query->getItem("id"))) {
        Scenario &s = Scenario::get(query->getItem("id"));
        result.insert("scenario", s.toJson());
        result.insert("success", true);
    } else {
       result.insert("msg", "Scenario Id invalid");
       result.insert("success", false);
    }

    loadJsonView(result);
}

void ScenarioController::jsonChangeStatus()
{
    QJsonObject result;

    if (!Authentification::auth().isConnected(header, cookie)) {
        result.insert("Result", "ERROR");
        result.insert("Message", "You are not logged.");
        loadJsonView(result);
        return;
    }

    if (!Scenario::isIdValid(query->getItem("id"))) {
        result.insert("Result", "ERROR");
        result.insert("Message", "Id not valid.");
        loadJsonView(result);
        return;
    }

    if (query->getItem("status") == "start" || query->getItem("status") == "stop") {
        result.insert("Result", "ERROR");
        result.insert("Message", "Status not valid.");
        loadJsonView(result);
        return;
    }

    Scenario scenario(query->getItem("id"));

    if (query->getItem("status") == "start") {
        scriptEngine->run("scenario_" + scenario.getId() + "_start");
    } else {
        scriptEngine->run("scenario_" + scenario.getId() + "_stop");
    }

    result.insert("Result", "OK");
    loadJsonView(result);
}
