#include "mysensors.h"
#include "libraries/settings.h"

#include <QDateTime>
#include <QtDebug>
#include <QSerialPortInfo>

MySensors::MySensors(QObject *parent) : QObject(parent)
{
    appendedString = "";
    currentPortTested = "";
    systemInError = false;
    settings = new Settings("mysensors", this);

    serial = new QSerialPort(this);
    serial->setBaudRate(QSerialPort::Baud115200);

    connectionTimer.setSingleShot(true);
    waitRegisterMsgTimer.setSingleShot(true);
    retryTimer.setSingleShot(true);

    connect(serial, &QSerialPort::readyRead, this, &MySensors::readData);
    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));
    connect(&connectionTimer, SIGNAL(timeout()), this, SLOT(connection()), Qt::QueuedConnection);
    connect(&waitRegisterMsgTimer, SIGNAL(timeout()), this, SLOT(connection()), Qt::QueuedConnection);
    connect(&retryTimer, SIGNAL(timeout()), this, SLOT(retryHandler()), Qt::QueuedConnection);
}

void MySensors::start()
{
    QString lastPort = settings->value("port");

    bool success = false;

    if (lastPort != "") {
       success = foundDevice(lastPort);
    }

    if (!success && !foundDevice()) {
        qCritical() << "mySensors: unable to connect to mySensors board";
        connectionTimer.start(8000);
    }
}

void MySensors::connection()
{
    systemInError = false;

    if (serial->isOpen()) {
        serial->close();
        qDebug() << "mySensors: disconnected because the registered message has not been send during the last 5 seconds";
    }

    waitRegisterMsgTimer.stop();

    if (!foundDevice()) {
        connectionTimer.start(17000);
    }
}

void MySensors::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError && systemInError == false) {
        qCritical() << "mySensors: board disconnected because" << qPrintable(serial->errorString());
        systemInError = true;

        waitRegisterMsgTimer.stop();

        if (!connectionTimer.isActive()) {
            connectionTimer.start(17000);
        }

        emit dataReceived("saveGateway", 0, 0, 0, "0");
    }
}

void MySensors::retryHandler()
{
    QMutableListIterator<RetryMsg> i(retryList);
    while (i.hasNext()) {
        RetryMsg *retryMsg = &i.next();

        if (retryMsg->lastSendTime.addMSecs(800) < QDateTime::currentDateTime()) {
            if (retryMsg->retryNumber == 0) {
                qWarning() << "mysensors: no ack reveived for the message" << qPrintable(retryMsg->msg);
                i.remove();
            } else {
                retryMsg->lastSendTime = QDateTime::currentDateTime();
                retryMsg->retryNumber--;
                send(retryMsg->msg, false, "Retry");
            }
        }
    }

    if (!retryList.isEmpty()) {
        retryTimer.start(50);
    }
}

void MySensors::readData()
{
    QByteArray data;

    while (serial->canReadLine()) {
       data = serial->readLine();
       QString msg = QString(data);

       if (waitRegisterMsgTimer.isActive()) {
           if (msg.contains("Gateway startup complete", Qt::CaseInsensitive)) {
                waitRegisterMsgTimer.stop();
                connectionTimer.stop();

                settings->setValue("port", currentPortTested);

                currentPortTested = "";

                qDebug() << "mySensors: registered with success!";
                appendData(msg);
                emit dataReceived("saveGateway", 0, 0, 0, "1");
           } else {
               qDebug() << "mySensors:" << qPrintable(msg);
           }
       } else {
           appendData(msg);
       }
    }
}

void MySensors::send(QString msg, bool checkAck, QString comment)
{
    if (serial->isOpen()) {
        if (settings->value("log", "info") == "debug" || settings->value("log", "info") == "info") {
            qDebug() << "mySensors: send" << qPrintable(msg) << qPrintable("(" + comment + ")");
        }
        QString msgToSend = msg + "\n";
        serial->write(msgToSend.toLatin1());

        if (checkAck && msg.split(";").length() > 3 && msg.split(";").at(3).toInt() == 1) {
            RetryMsg retryMsg;
            retryMsg.msg = msg;
            retryMsg.retryNumber = 5;
            retryMsg.lastSendTime = QDateTime::currentDateTime();
            retryList.append(retryMsg);

            if (!retryTimer.isActive()) {
                retryTimer.start(210);
            }
        }
    } else {
        qCritical() << "mySensors: not connected to send the message" << qPrintable(msg);
    }
}

bool MySensors::isConnected()
{
    return serial->isOpen();
}

bool MySensors::foundDevice(const QString port)
{
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (info.isBusy()) {
            continue;
        }

        if (port == "") {
            if (currentPortTested != "") {
                if (currentPortTested == info.portName()) {
                    currentPortTested = "";
                }
                continue;
            }

            if (info.portName().contains("ttyAMA0")) {
                continue;
            }
        } else {
            if (port != info.portName()) {
                continue;
            }
        }

        //qDebug() << "mySensors: try to connect on port" << qPrintable(info.portName());

        serial->setPort(info);
        if (serial->open(QIODevice::ReadWrite)) {
            currentPortTested = info.portName();
            waitRegisterMsgTimer.start(5000);
            qDebug() << "mySensors: connected on port" << qPrintable(info.portName()) << "(waiting of the registered message)";

            return true;
        }
    }

    currentPortTested = "";
    return false;
}

QString MySensors::encode(int destination, int sensor, int command, int acknowledge, int type, QString payload) {
    QString msg = QString::number(destination) + ";" + QString::number(sensor) + ";" + QString::number(command) + ";" + QString::number(acknowledge) + ";" + QString::number(type) + ";";
    msg += payload;

    return msg;
}

void MySensors::sendTime(int destination, int sensor) {
    QString payload = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
    QString td = encode(destination, sensor, C_INTERNAL, 0, I_TIME, payload);
    send(td);
}

void MySensors::sendConfig(int destination) {
    QString td = encode(destination, NODE_SENSOR_ID, C_INTERNAL, 0, I_CONFIG, "M");
    send(td);
}

void MySensors::appendData(QString str) {
    int pos=0;

    while (pos < str.length() && str.at(pos) != '\n') {
        appendedString = appendedString + str.at(pos);
        pos++;
    }

    if (pos < str.length() && str.at(pos) == '\n') {
        rfReceived(appendedString.trimmed());
        appendedString = "";
    }

    if (pos + 1 < str.length()) {
        appendData(str.mid(pos+1, str.length()-pos-1));
    }
}

void MySensors::rfReceived(QString data) {
    if (data != "" && data.split(";").length() > 4) {
        removeRetryMsg(data);

        // decoding message
        QStringList datas = data.split(";");
        int sender = datas.at(0).toInt();
        int sensor = datas.at(1).toInt();
        int command = datas.at(2).toInt();
        //int ack = datas.at(3).toInt();
        int type = datas.at(4).toInt();

        QString payload = "";
        if (datas.size() > 5) {
            payload = datas.at(5).trimmed();
        }

        if ((settings->value("log", "info") == "info" && type != I_LOG_MESSAGE) ||
                settings->value("log", "info") == "debug") {
            qDebug() << "mySensors:" << qPrintable(data);
        }

        switch (command) {
            case C_PRESENTATION:
                if (sensor == NODE_SENSOR_ID) {
                    //	saveProtocol(sender, payload); //arduino ou arduino relay
                } else {
                    emit dataReceived("saveSensor", sender, sensor, type, payload);
                    emit dataReceived("saveLibVersion", sender, sensor, type, payload);
                }
                break;
            case C_SET:
                emit dataReceived("saveValue", sender, sensor, type, payload);
                break;
            case C_REQ:
                emit dataReceived("getValue", sender, sensor, type, payload);
                break;
            case C_INTERNAL:
                switch (type) {
                    case I_BATTERY_LEVEL:
                        emit dataReceived("saveBatteryLevel", sender, sensor, type, payload);
                        break;
                    case I_TIME:
                        sendTime(sender, sensor);
                        break;
                    case I_VERSION:
                        emit dataReceived("saveLibVersion", sender, sensor, type, payload);
                        break;
                    case I_ID_REQUEST:
                        emit dataReceived("getNextSensorId", sender, sensor, type, payload);
                        break;
                    case I_ID_RESPONSE:
                        break;
                    case I_INCLUSION_MODE:
                        break;
                    case I_CONFIG:
                        sendConfig(sender);
                        break;
                    case I_PING:
                        break;
                    case I_PING_ACK:
                        break;
                    case I_LOG_MESSAGE:
                        break;
                    case I_CHILDREN:
                        break;
                    case I_SKETCH_NAME:
                        emit dataReceived("saveSketchName", sender, sensor, type, payload);
                        break;
                    case I_SKETCH_VERSION:
                        emit dataReceived("saveSketchVersion", sender, sensor, type, payload);
                        break;
                    case I_REBOOT:
                        break;
                    case I_PONG:
                        emit dataReceived("saveValue", sender, sensor, type, payload);
                    default:
                        break;
                }
                break;
            case C_STREAM:
                break;
            default:
                break;
        }
    }
}

void MySensors::removeRetryMsg(QString msg)
{
    QMutableListIterator<RetryMsg> i(retryList);
    while (i.hasNext()) {
        if (i.next().msg == msg) {
            i.remove();
        }
    }
}

