#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <QUrl>
#include <cstdlib>

#include "OTAClientController.h"

int main(int argc, char *argv[])
{
    setenv("COMMONAPI_DEFAULT_BINDING", "someip", 1);
    setenv("COMMONAPI_CONFIG",          "/etc/commonapi.ini", 1);
    setenv("VSOMEIP_APPLICATION_NAME",  "OTAClientUI", 1);
    setenv("VSOMEIP_CONFIGURATION",     "/etc/vsomeip-client.json", 1);

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    OTAClientController *controller = new OTAClientController(&app);
    engine.rootContext()->setContextProperty("controller", controller);

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/OTAUpdater/Main.qml")));
    if (engine.rootObjects().isEmpty()) return -1;

    QTimer::singleShot(0, controller, &OTAClientController::initialize);

    return app.exec();
}