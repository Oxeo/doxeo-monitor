#include "defaultcontroller.h"
#include "libraries/authentification.h"
#include "libraries/messagelogger.h"
#include "libraries/device.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QTime>
#include <QTimer>
#include <QHostAddress>

DefaultController::DefaultController()
{
    router.insert("stop", "stopApplication");
    router.insert("logs", "logs");
    router.insert("logs.js", "jsonLogs");
    router.insert("clear_logs.js", "jsonClearLogs");
    router.insert("system.js", "jsonSystem");
}

void DefaultController::defaultAction()
{   
    if (!Authentification::auth().isConnected(header, cookie)) {
        redirect("/auth");
        return;
    }

    QHash<QString, QByteArray> view;
    view["head"] = loadHtmlView("views/default/default.head.html", NULL, false);
    view["content"] = loadHtmlView("views/default/default.body.html", NULL, false);
    view["bottom"] = loadHtmlView("views/default/default.js", NULL, false);
    loadHtmlView("views/template.html", &view);
}

void DefaultController::stop()
{

}

void DefaultController::stopApplication()
{
    QJsonObject result;

    if (Authentification::auth().isConnected(header, cookie) ||
            socket->peerAddress() == QHostAddress::LocalHost)
    {
       QTimer::singleShot(100, QCoreApplication::instance(), SLOT(quit()));
       result.insert("success", true);
    } else {
        result.insert("msg", "You are not logged.");
        result.insert("success", false);
    }

    loadJsonView(result);
}

void DefaultController::logs()
{
    if (!Authentification::auth().isConnected(header, cookie)) {
        redirect("/auth");
        return;
    }

    QHash<QString, QByteArray> view;
    view["content"] = loadHtmlView("views/default/logs.html", NULL, false);
    view["bottom"] = loadHtmlView("views/default/logs.js", NULL, false);
    loadHtmlView("views/template.html", &view);
}

void DefaultController::jsonLogs()
{
    QJsonObject result;
    QString request = query->getItem("type");

    if (!Authentification::auth().isConnected(header, cookie)) {
        result.insert("msg", "You are not logged.");
        result.insert("success", false);
        loadJsonView(result);
        return;
    }
    
    QList<MessageLogger::Log> &logs = MessageLogger::logger().getMessages();
    QJsonArray jsonArray;

    foreach (const MessageLogger::Log &log, logs) {
        if (request == "warning" && log.type != "warning" && log.type != "critical") {
            continue;
        } else if (request == "critical" && log.type != "critical") {
            continue;
        } else {
            QJsonObject object;
            object.insert("id", log.id);
            object.insert("date", log.date.toString("dd/MM/yyyy hh:mm:ss"));
            object.insert("message", log.message);
            object.insert("type", log.type);
            jsonArray.push_back(object);
        }
    }

    result.insert("messages", jsonArray);
    result.insert("success", true);
    loadJsonView(result);
}

void DefaultController::jsonClearLogs()
{
    QJsonObject result;

    if (!Authentification::auth().isConnected(header, cookie)) {
        result.insert("msg", "You are not logged.");
        result.insert("success", false);
        loadJsonView(result);
        return;
    }
    
    bool ok = false;
    int id = query->getItem("id").toInt(&ok, 10);
    
    if (!ok || id < 0) {
        result.insert("success", false);
        result.insert("msg", "ID param is not a number!");
        loadJsonView(result);
        return;
    }
    
    if (query->getItem("type") == "") {
        result.insert("success", false);
        result.insert("msg", "TYPE param is missing!");
        loadJsonView(result);
        return;
    }
    
    MessageLogger::logger().removeBeforeId(id, query->getItem("type"));
    result.insert("success", true);

    loadJsonView(result);
}

void DefaultController::jsonSystem()
{
    QJsonObject result;
    result.insert("success", true);
    result.insert("time", QTime::currentTime().toString("HH:mm"));
    result.insert("device_connected", Device::Instance()->isConnected());
    loadJsonView(result);
}

