#include <QQuickStyle>

#include "Config/config.h"

#include "QDateTime"
#include "QDebug"
#include "QSurfaceFormat"
#include "QTranslator"

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle("Imagine");
    int exitCode = 0;
    do {
//        qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

        QApplication app(argc, argv);
        QSurfaceFormat format;
        format.setSamples(8);
        QSurfaceFormat::setDefaultFormat(format);

        QQmlApplicationEngine engine;
        Config *config = new Config(&engine);
        config->init(&app, &engine);

        // 多语言翻译
        // 必须在这里加载
        // 否则即使installTranslator 返回true
        // 也不会翻译成中文
        QTranslator ts;
        switch(config->language())
        {
        case Config::__cn: {
            if( ts.load(":/translation/Cn.qm") ) {
                if( app.installTranslator(&ts) ) {
                    qDebug() << "load cn transloation";
                }
                else {
                    qDebug() << "install translator fail";
                }
            }
        }
            break;
        case Config::__en: {
        }
            break;
        default:
            break;
        }

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
