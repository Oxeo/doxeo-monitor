#include "urlquery.h"
#include <QUrl>

UrlQuery::UrlQuery(QString url)
{
    url.remove(QRegExp("^/+")); //remove double slash for apache proxy
    query = url.split("/");

    if (query.last().contains("?")) {
        QString text = query.last();
        text.remove(QRegExp("^.*\\?"));
        QStringList list = text.split("&");

        foreach(QString element, list) {
            QStringList a = element.split("=");
            if (a.length() > 1) {
                this->item.insert(normalize(a.at(0)), normalize(a.at(1)));
            } else {
                this->item.insert(normalize(a.at(0)), "");
            }
        }

        query.last().remove(QRegExp("\\?.*$"));
    }
}

QString UrlQuery::getQuery(int element)
{
    if (query.length() > element) {
        return query.at(element);
    } else {
        return "";
    }
}

QStringList UrlQuery::getAllQuery()
{
    return query;
}

QString UrlQuery::getItem(QString key)
{
    if (item.contains(key)) {
        return item[key];
    } else {
        return "";
    }
}

QString UrlQuery::normalize(const QString &text)
{
    QString input = "http://www.test.com/" + text;
    input.replace("+", " ");
    QByteArray latin = input.toLatin1();
    QByteArray utf8 = input.toUtf8();
    QString result;
    if (latin != utf8) {
       // URL string containing unicode characters (no percent encoding expected)
       result = QUrl::fromUserInput(input).toDisplayString().remove("http://www.test.com/");
    } else {
       // URL string containing ASCII characters only (assume possible %-encoding)
       result = QUrl::fromUserInput(QUrl::fromPercentEncoding(input.toLatin1())).toDisplayString().remove("http://www.test.com/");
    }

    result.replace("%22", "\"");
    result.replace("%7B", "{");
    result.replace("%7D", "}");
    result.replace("%3C", "<");
    result.replace("%3E", ">");
    result.replace("%5C", "\\");
    result.replace("%7C", "|");
    result.replace("%0D%0A", "\n");
    result.replace("%0A", "\n");

    return result;
}

