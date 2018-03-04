#include "doxeomonitor.h"

#include "models/user.h"
#include "models/heater.h"
#include "controllers/switchcontroller.h"
#include "controllers/defaultcontroller.h"
#include "controllers/assetcontroller.h"
#include "controllers/authcontroller.h"
#include "controllers/thermostatcontroller.h"
#include "controllers/sensorcontroller.h"
#include "controllers/scriptcontroller.h"
#include "libraries/messagelogger.h"
#include "libraries/authentification.h"
#include "libraries/device.h"
#include "libraries/firebasecloudmessaging.h"
#include "core/database.h"

#include <QDir>
#include <iostream>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTextStream>
#include <QtDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <stdio.h>
#include <stdlib.h>

DoxeoMonitor::DoxeoMonitor(int &argc, char **argv) : QCoreApplication(argc, argv)
{
    setOrganizationName("Oxeo");
    setOrganizationDomain("www.oxeo.fr");
    setApplicationName("Doxeo Monitor");
    setApplicationVersion("V1.2.0");

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(closeApplication()));
}

int DoxeoMonitor::start()
{
    QSettings settings;
    bool error;
    
    // Command line parser
    if (commandLineParser(&error)) {
        return (error?-1:0);
    }

    // Start Http server
    httpServer = new HttpServer(8080, new DefaultController(), this);
    if (!httpServer->isListening()) {
        qCritical() << applicationName() + " stopped: http server already running";
        return -1;
    }

    // Connection to Mysql Database
    Database::initialize("QMYSQL");
    if(!Database::open()) {
        qCritical() << applicationName() + " stopped: Mysql connection failed";
        return -1;
    }
    
    // Initialize Firebase Cloud Messaging
    FirebaseCloudMessaging *fcm = new FirebaseCloudMessaging(settings.value("firebasecloudmessaging/projectname", "doxeo").toString(), this);
    fcm->setServerKey(settings.value("firebasecloudmessaging/serverkey", "").toString());
    MessageLogger::logger().setFirebaseCloudMessaging(fcm);

    // Initialize logger messages
    if (!verbose) {
        qInstallMessageHandler(MessageLogger::messageHandler);
    }

    // Connect device
    Device::initialize("Doxeoboard", this);

    // Add controller
    httpServer->addController(new AssetController(), "assets");
    httpServer->addController(new SwitchController(this), "switch");
    httpServer->addController(new SensorController(this), "sensor");
    httpServer->addController(new AuthController(this), "auth");
    httpServer->addController(new ThermostatController(this), "thermostat");
    httpServer->addController(new ScriptController(this), "script");

    qDebug() << QCoreApplication::applicationName() + " started.";
    return this->exec();
}

void DoxeoMonitor::closeApplication()
{
    qInstallMessageHandler(0);
    qDebug() <<applicationName() + " stopped by user.";

    QHash<QString, AbstractController *> controllers = httpServer->getControllers();
    foreach (AbstractController *ctrl, controllers) {
        ctrl->stop();
    }
}

QString DoxeoMonitor::commandLine(QString title)
{
    QTextStream Qin(stdin);
    QTextStream cout(stdout);
    QString result;

    cout << title << endl;
    forever
    {
        QString line = Qin.readLine();
        if (!line.isNull())
        {
            if (line != "") {
                result = line;
            }
            break;
        }
    }

    return result;
}

void DoxeoMonitor::configure()
{
    QSettings settings;
    QString userInput;
    QString oldValue;

    oldValue = settings.value("database/hostname", "").toString();
    userInput = commandLine("Enter database host name (" + oldValue + "):");
    if (userInput != "") {
        settings.setValue("database/hostname", userInput);
    }

    oldValue = settings.value("database/username", "").toString();
    userInput = commandLine("Enter database username (" + oldValue + ")");
    if (userInput != "") {
        settings.setValue("database/username", userInput);
    }

    oldValue = settings.value("database/password", "").toString();
    userInput = commandLine("Enter database password (" + oldValue + "):");
    if (userInput != "") {
        settings.setValue("database/password", userInput);
    }

    oldValue = settings.value("database/databasename", "").toString();
    userInput = commandLine("Enter database name (" + oldValue + "):");
    if (userInput != "") {
        settings.setValue("database/databasename", userInput);
    }
    
    oldValue = settings.value("firebasecloudmessaging/projectname", "").toString();
    userInput = commandLine("Enter Firebase Cloud Messaging project name (" + oldValue + "):");
    if (userInput != "") {
        settings.setValue("firebasecloudmessaging/projectname", userInput);
    }
    
    oldValue = settings.value("firebasecloudmessaging/serverkey", "").toString();
    userInput = commandLine("Enter Firebase Cloud Messaging server key (" + oldValue + "):");
    if (userInput != "") {
        settings.setValue("firebasecloudmessaging/serverkey", userInput);
    }
}

void DoxeoMonitor::createUser()
{
    QTextStream cout(stdout);
    QString userName;
    QString password;
    User newUser = User();

    // Connection to Mysql Database
    Database::initialize("QMYSQL");
    if(!Database::open()) {
        cout << "Mysql connection failed!";
        return;
    }

    userName = commandLine("email:");
    password = commandLine("password:");

    if (userName != "" && password != "") {
        newUser.setUsername(userName);
        newUser.setPassword(password);

        if (newUser.flush()) {
            cout << "User created with success!" << endl;
            cout << "The application shall be restared to take into acount the new user!" << endl;
        } else {
            cout << "User not created: unknown error!" << endl;
        }
    } else {
        cout << "User not created: email or password empty!" << endl;
    }
}

bool DoxeoMonitor::commandLineParser(bool *error)
{
    *error = false;

    QCommandLineParser parser;
    parser.setApplicationDescription("Doxeo-monitor helper");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption configureOption(QStringList() << "c" << "configure", "Configure the programme.");
    parser.addOption(configureOption);

    QCommandLineOption userOption(QStringList() << "u" << "user", "create a new user");
    parser.addOption(userOption);

    QCommandLineOption debugOption(QStringList() << "d" << "debug", "Print debug logs in stderr/console.");
    parser.addOption(debugOption);

    QCommandLineOption stopOption(QStringList() << "q" << "quit", "Quit the application.");
    parser.addOption(stopOption);

    QCommandLineOption applicationPathOption(QStringList() << "p" << "path",
                                         "Set the absolute path of the application directory.", "path");
    parser.addOption(applicationPathOption);

    parser.process(*this);

    if (parser.isSet(configureOption)) {
        configure();
        return true;
    }

    if (parser.isSet(userOption)) {
        createUser();
        return true;
    }

    if (parser.isSet(stopOption)) {
        if (!stopApplication()) {
            *error = true;
        }
        return true;
    }

    if (parser.value(applicationPathOption) != "") {
        QDir::setCurrent(parser.value(applicationPathOption));
    }

    verbose = parser.isSet(debugOption);

    return false;
}

bool DoxeoMonitor::stopApplication()
{
    QEventLoop eventLoop;
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    QNetworkRequest req(QUrl(QString("http://127.0.0.1:8080/stop")));
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json= doc.object();

        if (json.contains("success") && json.value("success").toBool(false)) {
            delete reply;
            return true;
        } else {
            qCritical() << "Error: " << json.value("msg").toString();
        }
    }
    else {
        qCritical() << "Error: " <<reply->errorString();
    }

    delete reply;
    return false;
}
