#ifndef MESSAGELOGGER_H
#define MESSAGELOGGER_H

#include "firebasecloudmessaging.h"
#include <QList>
#include <QMessageLogContext>
#include <QString>
#include <QDateTime>

class MessageLogger
{
public:
    struct Log{
        int id;
        QDateTime date;
        QString message;
        QString type;
    };

    QList<Log>& getMessages();
    void addMessage(QString type, QString msg);
    void removeBeforeId(int id, QString type);
    void setFirebaseCloudMessaging(FirebaseCloudMessaging *fcm);
    FirebaseCloudMessaging* getFCM();

    static MessageLogger& logger();
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

protected:
    MessageLogger();
    ~MessageLogger();

    QList<Log> messages;
    int idCpt;
    FirebaseCloudMessaging *fcm;
};

#endif // MESSAGELOGGER_H
