#include <QQuickStyle>

#include "CameraView/cameraview.h"
#include "Config/config.h"

#include "QDebug"
#include "QSurfaceFormat"

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle("Imagine");

    int exitCode = 0;
    do {
//        qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#else
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

//        qmlRegisterType<CameraView>("custom.cameraview", 0, 1, "CameraView");

        QGuiApplication app(argc, argv);

        QSurfaceFormat format;
        format.setSamples(8);
        QSurfaceFormat::setDefaultFormat(format);

        QQmlApplicationEngine engine;
        Config *config = new Config(&engine);
        config->init(&app, &engine);

        const QUrl url(QStringLiteral("qrc:/main.qml"));
        QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                         &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

        engine.load(url);

        exitCode = app.exec();

        config->release();
        qDebug() << "app exit";
    } while(exitCode == REBOOT_CODE);

    return exitCode;
}
