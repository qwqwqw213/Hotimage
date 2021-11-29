#include <QQuickStyle>

#include "Config/config.h"

#include "QDateTime"
#include "QDebug"
#include "QSurfaceFormat"
#include "QTranslator"
#include "QFontDatabase"

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle("Imagine");

    int exitCode = 0;
    do {
//        qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

        QApplication app(argc, argv);

        QFontDatabase fd;
        int fontIndex = fd.addApplicationFont(":/font/SourceHanSansCN-Normal.otf");
        if( fontIndex >= 0 ) {
            QFont font = app.font();
            QString family = QFontDatabase::applicationFontFamilies(fontIndex).at(0);
            font.setFamily(family);
            qDebug() << "set font family:" << family;
            app.setFont(font);
        }
        fd.addApplicationFont(":/font/fontawesome-webfont.ttf");

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
    } while(exitCode == REBOOT_CODE);

    return exitCode;
}
