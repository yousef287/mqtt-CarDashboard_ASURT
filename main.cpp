#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <Controllers/udpclient.h>
#include <Controllers/mqttclient.h>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    UdpClient udpClient;
    MqttClient mqttClient;

    engine.rootContext()->setContextProperty("udpClient", &udpClient);
    engine.rootContext()->setContextProperty("mqttClient", &mqttClient);


    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Car_Dashboard", "Main");

    return app.exec();
}
