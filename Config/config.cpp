#include "config.h"

#include "ImageListModel/imagelistmodel.h"
#include "TcpCamera/tcpcamera.h"
#include "AndroidInterface/androidinterface.h"

#include "QDebug"
#include "QScreen"
#include "QQmlContext"
#include "QStandardPaths"
#include "QSettings"
#include "QDateTime"
#include "QFontDatabase"
#include "QThread"

#ifdef Q_OS_ANDROID
#include "MadgwickAHRS.hpp"
#include "QAccelerometer"
#include "QGyroscope"
#include "thread"
#endif

class ConfigPrivate
{
public:
    ConfigPrivate(Config *parent);
    ~ConfigPrivate();

    int width;
    int height;

    int oldRotation;
#ifdef Q_OS_ANDROID
    MadgwickAHRS *madgwick;

    QAccelerometer *accelerometer;
    QGyroscope *gyroscope;

    qint64 old_ts;
    qreal accelerometer_x;
    qreal accelerometer_y;
    qreal accelerometer_z;
#endif

    AndroidInterface *androidInterface;

    QScopedPointer<ImageListModel> imageModel;
    QScopedPointer<TcpCamera> tcpCamera;

    QScreen *screen;

    void readSetting();
    void saveSetting();

private:
    Config *f;
};

Config::Config(QObject *parent)
    : QObject(parent)
    , p(new ConfigPrivate(this))
{

}

Config::~Config()
{

}

int Config::init(QGuiApplication *gui, QQmlApplicationEngine *e)
{
    // 读配置文件
    p->readSetting();

    p->screen = QGuiApplication::primaryScreen();
    p->screen->setOrientationUpdateMask(Qt::PortraitOrientation |
                                     Qt::LandscapeOrientation |
                                     Qt::InvertedLandscapeOrientation |
                                     Qt::InvertedPortraitOrientation);
    QObject::connect(p->screen, static_cast<void (QScreen::*)(Qt::ScreenOrientation)>(&QScreen::orientationChanged),
                     [=](Qt::ScreenOrientation orientation){
        qDebug() << "orientationChanged screen:" << orientation << p->screen->availableGeometry();
    });

    QRect r = p->screen->availableGeometry();
#ifdef Q_OS_ANDROID
    p->width = r.width();
    p->height = r.height();
#else
    p->width = 960;
    p->height = 540;
#endif
    qDebug() << QThread::currentThreadId() << "windows size:" << p->width << p->height;

    e->rootContext()->setContextProperty("Config", this);

    //    qmlRegisterType<TcpCamera>("tcpcamera", 1, 0, "TcpCamera");

    p->imageModel.reset(new ImageListModel);
#ifdef Q_OS_ANDROID
    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if( !path.isEmpty() ) {
        path.append("/Hotimage");
        p->imageModel->search(path);
    }
#else
    p->imageModel->search("C:\\Users\\DELL\\Desktop\\train");
#endif
    e->rootContext()->setContextProperty("imageModel", p->imageModel.data());

    p->tcpCamera.reset(new TcpCamera);
    ImageProvider *provider = p->tcpCamera->provider();
    e->addImageProvider(provider->url(), provider);
    e->rootContext()->setContextProperty("tcpCamera", p->tcpCamera.data());

    QObject::connect(p->tcpCamera.data(), static_cast<void (TcpCamera::*)(const QString &)>(&TcpCamera::captureFinished),
                     p->imageModel.data(), &ImageListModel::add);
    p->tcpCamera->open();

    p->androidInterface = new AndroidInterface(this);

    QFontDatabase fd;
    int fontIndex = fd.addApplicationFont(":/font/fontawesome-webfont.ttf");
    if( fontIndex >= 0 ) {
        qDebug() << fd.applicationFontFamilies(fontIndex);
    }
    return 1;
}



int Config::width()
{
    return p->width;
}

int Config::height()
{
    return p->height;
}

QString Config::albumFolder()
{
#ifdef Q_OS_ANDROID
    QString path = QString("file://%1/HotImage/")
            .arg(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    return path;
#else
    return QString("file:///C:/Users/DELL/Desktop/train/");
#endif
}

void Config::release()
{
//    p->tcpCamera.reset();
//    p->imageModel.reset();
}

qreal Config::rotation()
{
    return p->oldRotation;
}

void Config::setRotation()
{
#ifndef Q_OS_ANDROID
    p->oldRotation += 90;
    if( p->oldRotation > 270 ) {
        p->oldRotation = 0;
    }
    emit rotationChanged();
#endif
}

ConfigPrivate::ConfigPrivate(Config *parent)
{
    f = parent;

    oldRotation = 0;
#ifdef Q_OS_ANDROID
    accelerometer_x = 0.0;
    accelerometer_y = 0.0;
    accelerometer_z = 0.98;
    madgwick = new MadgwickAHRS(0.2, 50);
    accelerometer = new QAccelerometer;
    QObject::connect(accelerometer, &QAccelerometer::readingChanged, [=](){
        QAccelerometerReading *r = accelerometer->reading();
//        accelerometer_x = r->x();
//        accelerometer_y = r->y();
//        accelerometer_z = r->z();
        qreal x = r->x();
        qreal y = r->y();
        int rotation = oldRotation;
        if (x > -2.5 && x <= 2.5 && y > 7.5 && y <= 10 && (rotation != 270 && rotation != -90)) {
            if( rotation == 0 || rotation == -180 ) {
                rotation = -90;
            }
            else {
                rotation = rotation == -270 ? -90 : 270;
            }
        }
        else if (x > 7.5 && x <= 10 && y > -2.5 && y <= 2.5 && (rotation != 0 && rotation != -360)) {
            if( rotation == -270 ) {
                rotation = -360;
            }
            else {
                rotation = 0;
            }
        }
        else if (x > -2.5 && x <= 2.5 && y > -10 && y <= -7.5 && (rotation != 90 && rotation != -270)) {
            if( rotation == -180 || rotation == -360 ) {
                rotation = -270;
            }
            else {
                rotation = rotation == -90 ? -270 : 90;
            }
        }
        else if (x > -10 && x <= -7.5 && y > -2.5 && y < 2.5 && (rotation != 180 && rotation != -180)) {
            if( rotation == -90 || rotation == -270 ) {
                rotation = -180;
            }
            else {
                rotation = 180;
            }
        }

        if( rotation != oldRotation ) {
            oldRotation = rotation;
            qDebug() << "rotation changed:" << oldRotation;
            emit f->rotationChanged();
        }
    });
    gyroscope = new QGyroscope;
    QObject::connect(gyroscope, &QGyroscope::readingChanged, [=](){
        QGyroscopeReading *r = gyroscope->reading();
        qint64 ts = QDateTime::currentDateTime().toMSecsSinceEpoch();
        qreal interval = ts - old_ts;
        old_ts = ts;
        madgwick->MadgwickAHRSIMUApply(r->x(), r->y(), r->z(),
                                       accelerometer_x / 10.0, accelerometer_y / 10.0, accelerometer_z / 10.0,
                                       interval / 1000.0);

        float roll;
        float pitch;
        float yaw;
        madgwick->MadgwickComputeAngles(roll, pitch, yaw);
        qDebug() << "roll:" << roll * 60 << "pitch:" << pitch * 60 << "yaw:" << yaw * 60
                 << r->x() << r->y() << r->z()
                 << accelerometer_x << accelerometer_y << accelerometer_z
                 << interval;
    });

    old_ts = QDateTime::currentDateTime().toMSecsSinceEpoch();
    accelerometer->start();
//    gyroscope->start();

#endif
}

ConfigPrivate::~ConfigPrivate()
{
#ifdef Q_OS_ANDROID
    accelerometer->stop();
    gyroscope->stop();
    accelerometer->deleteLater();
    gyroscope->deleteLater();
    delete madgwick;
#endif

    saveSetting();
}

void ConfigPrivate::readSetting()
{
#ifdef Q_OS_ANDROID
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QString("/");
#else
    QString path = QGuiApplication::applicationDirPath() + QString("/setting.ini");
#endif
    qDebug() << "read setting path:" << path;
    QSettings *s = new QSettings(path, QSettings::IniFormat);


    delete s;
    s = NULL;
}

void ConfigPrivate::saveSetting()
{
#ifdef Q_OS_ANDROID
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QString("/");
#else
    QString path = QGuiApplication::applicationDirPath() + QString("/setting.ini");
#endif
    QSettings *s = new QSettings(path, QSettings::IniFormat);



    s->sync();
    delete s;
    s = NULL;

    qDebug() << "save setting path:" << path;
}