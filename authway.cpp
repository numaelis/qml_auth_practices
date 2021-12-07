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

#include "authway.h"
#include <QEventLoop>
#include <QDesktopServices>
AuthWay::AuthWay(QObject *parent) : QObject(parent)
{
    countAuth = 0;
    connect(&google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            this, &AuthWay::slotUrl);
//    connect(&google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);
    connect(&google, &QOAuth2AuthorizationCodeFlow::error,
            this, &AuthWay::error);
    connect(&google, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](
            QAbstractOAuth::Status status) {

        if (status == QAbstractOAuth::Status::Granted){
            countAuth = 0;
            emit this->granted();
            qDebug()<<"Granted";
            QSettings s;
            s.beginGroup("OAuth2");
            s.setValue("token", google.token());
            s.setValue("expiration", google.expirationAt());
            s.endGroup();
        }
        if (status == QAbstractOAuth::Status::NotAuthenticated){
            qDebug()<<"NotAuthenticated";
            emit notAuthenticated();
            emit notGranted();
        }
        if (status == QAbstractOAuth::Status::RefreshingToken){
            qDebug()<<"RefreshingToken";
            emit notGranted();
        }
        if (status == QAbstractOAuth::Status::TemporaryCredentialsReceived){
            qDebug()<<"TemporaryCredentialsReceived";
            if(countAuth < 3){
                google.grant();
                countAuth+=1;
            }else{
                emit notGranted();
            }

        }
        emit this->statusChanged(status);
    });

    google.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QVariantMap* parameters)
    {
        if (stage == QAbstractOAuth::Stage::RequestingAccessToken) {
            //thank to https://appfluence.com/productivity/how-to-authenticate-qt-google-sso/
            QByteArray code = parameters->value("code").toByteArray();
            (*parameters)["code"] = QUrl::fromPercentEncoding(code);
        }
        if (stage == QAbstractOAuth::Stage::RefreshingAccessToken) {
            parameters->insert("client_id" ,google.clientIdentifier());
            parameters->insert("client_secret", google.clientIdentifierSharedKey());
        }
    });

}

void AuthWay::loadCredentials(const QJsonObject &credentials)
{
    const auto settingsObject = credentials["web"].toObject();
    const QUrl authUri(settingsObject["auth_uri"].toString());
    const auto clientId = settingsObject["client_id"].toString();
    const QUrl tokenUri(settingsObject["token_uri"].toString());
    const auto clientSecret(settingsObject["client_secret"].toString());
    const auto redirectUris = settingsObject["redirect_uris"].toArray();
    const QUrl redirectUri(redirectUris[0].toString());
    const auto port = static_cast<quint16>(redirectUri.port());


    google.setAuthorizationUrl(authUri);
    google.setClientIdentifier(clientId);
    google.setAccessTokenUrl(tokenUri);
    google.setClientIdentifierSharedKey(clientSecret);

    replyHandler = new QOAuthHttpServerReplyHandler(port, this);
    google.setReplyHandler(replyHandler);

}

void AuthWay::initAuth()
{
    QSettings s;
    s.beginGroup("OAuth2");
    QDateTime expiration = s.value("expiration").toDateTime();
    if(expiration > QDateTime::currentDateTime()){
        qDebug()<<"there is token"<<s.value("token").toString();
        google.setRefreshToken(s.value("token").toString());
        google.refreshAccessToken();
    }
    else{
        google.grant();
    }
    s.endGroup();
}

void AuthWay::setScope(const QString &scope)
{
    google.setScope(scope);
}

QJsonObject AuthWay::getREST(const QString &strres)
{
    QJsonObject result = _getREST(strres);
    if(result.contains("error")){
        if(result.value("error").toObject().value("code").toInt()==401){
            initAuth();
            uint count = 0;
            bool waiting = isGranted();
            while (!waiting) {
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                count++;
                waiting = isGranted();
                QThread::msleep(1);
                if (count > 1000 * 5) {/// wait 5 seconds
                    waiting = true;
                }
            }
            result = _getREST(strres);
            if(result.contains("error")){
                if(result.value("error").toObject().value("code").toInt()==401){
                    emit notAuthenticated();
                }
            }
        }
    }
    return result;
}

QJsonObject AuthWay::_getREST(const QString &strres)
{
    QEventLoop connection_loop;
    QNetworkReply* reply;
    reply = google.get(QUrl(strres));
    connect(reply, SIGNAL( finished() ), &connection_loop, SLOT( quit() ) );
    connection_loop.exec();
    reply->deleteLater();
    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonObject resultObject;
    const auto document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error) {

    }else{
        if(document.isObject()){
            return document.object();
        }
    }
    return QJsonObject();
}

bool AuthWay::isGranted()
{
    if(google.status()==QAbstractOAuth::Status::Granted){
        if(QDateTime::currentDateTime() < google.expirationAt()){
            return true;
        }else{
            return false;
        }

    }else{
        return false;
    }
}



void AuthWay::slotUrl(const QUrl &url)
{
    emit urlChanged(url);
}

void AuthWay::error(const QString &error, const QString &errorDescription, const QUrl &uri)
{
    qDebug()<<errorDescription;
}
