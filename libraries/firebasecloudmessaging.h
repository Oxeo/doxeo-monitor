#ifndef FIREBASECLOUDMESSAGING_H
#define FIREBASECLOUDMESSAGING_H

#include <QObject>
#include <QJsonObject>
#include <QNetworkAccessManager>

class FirebaseCloudMessaging : public QObject
{
    Q_OBJECT

public:
    struct Message {
      QString type;
      QString title;
      QString body;
    };

    explicit FirebaseCloudMessaging(QString projectName, QObject *parent = 0);
    void setServerKey(QString serverKey);
    void send(Message message);

public slots:
    void networkReply(QNetworkReply*);

protected:
    QJsonObject buildJsonMessage(Message message);

    QNetworkAccessManager *manager;
    QString projectName;
    QString serverKey;
    Message message;
};

#endif // FIREBASECLOUDMESSAGING_H
