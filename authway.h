#ifndef AUTHWAY_H
#define AUTHWAY_H
/****************************************************************************
** __author__ = "Numael Garay"
**__copyright__ = "Copyright 2021"
**__license__ = "MIT License"
**__version__ = "1.0"
**__docs__ = ["https://doc.qt.io/qt-5/qtnetworkauth-index.html",
**            "https://www.qt.io/blog/2017/01/25/connecting-qt-application-google-services-using-oauth-2-0",
**            "https://stackoverflow.com/questions/48453550/how-to-create-a-login-page-using-qt-oauth",
**            "https://appfluence.com/productivity/how-to-authenticate-qt-google-sso/",
**            "https://forum.qt.io/topic/78926/saving-restoring-oauth2-tokens/15"]
****************************************************************************/
#include <QObject>
#include <QtNetworkAuth>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

class AuthWay: public QObject
{
    Q_OBJECT
public:
    explicit AuthWay(QObject *parent = nullptr);
    Q_INVOKABLE void loadCredentials(const QJsonObject &credentials);
    Q_INVOKABLE void initAuth();
    Q_INVOKABLE void setScope(const QString &scope);
    Q_INVOKABLE QJsonObject getREST(const QString &strres);
    Q_INVOKABLE QJsonObject _getREST(const QString &strres);
    Q_INVOKABLE bool isGranted();

private:
    QOAuth2AuthorizationCodeFlow google;
    QOAuthHttpServerReplyHandler * replyHandler;
    int countAuth;

signals:
    void urlChanged(QUrl url);
    void statusChanged(QAbstractOAuth::Status status);
    void granted();
    void notGranted();
    void notAuthenticated();

public slots:
    void slotUrl(const QUrl &url);
    void error(const QString &error, const QString &errorDescription, const QUrl &uri);
};

#endif // AUTHWAY_H
