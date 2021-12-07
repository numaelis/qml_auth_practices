#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "authway.h"
#include <QtWebEngine>


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
#endif
    QtWebEngine::initialize();
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    AuthWay* myaut = new AuthWay(nullptr);
    engine.rootContext()->setContextProperty("AuthGateway", myaut);
    const QUrl url(QStringLiteral("qrc:/main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
