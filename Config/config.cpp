#include "config.h"

#include "ImageListModel/imagelistmodel.h"
#include "TcpCamera/tcpcamera.h"
#include "VideoPlayer/videoplayer.h"

#include "QDebug"
#include "QScreen"
#include "QQmlContext"
#include "QStandardPaths"
#include "QSettings"
#include "QDateTime"
#include "QFontDatabase"
#include "QThread"
#include "QTranslator"
#include "QFileInfo"
#include "QDir"

#ifndef Q_OS_WIN32
#include "QAccelerometer"
#include "QGyroscope"
#include "thread"
#endif

#ifdef Q_OS_IOS
#include "IOSInterface/iosinterface.h"
#elif defined (Q_OS_ANDROID)
#include "AndroidInterface/androidinterface.h"
#endif

#define PROGRAM_VERSION             "1.0.0.1"

class ConfigPrivate
{
public:
    ConfigPrivate(Config *parent);
    ~ConfigPrivate();

    QQmlApplicationEngine *e;

    int width;
    int height;

    int oldRotation;
#ifndef Q_OS_WIN32
    QAccelerometer *accelerometer;
    QGyroscope *gyroscope;

    qint64 old_ts;
    qreal accelerometer_x;
    qreal accelerometer_y;
    qreal accelerometer_z;
#endif

    QScopedPointer<ImageListModel> imageModel;
    QScopedPointer<TcpCamera> tcpCamera;
    QScopedPointer<VideoPlayer> videoPlayer;
#if defined (Q_OS_IOS)
    QScopedPointer<IOSInterface> iosInterface;
#elif defined (Q_OS_ANDROID)
    QScopedPointer<AndroidInterface> androidInterface;
#endif

    QScreen *screen;

    int language;
    QString documentsPath;
    QString imageGalleryPath;

    bool isLandscape;

    QSettings *setting;
    void readSetting();
    void saveSetting();

    QTranslator *ts;

    int leftMargin;
    int rightMargin;
    int topMargin;
    int bottomMargin;

    QStringList fontFamilies;
    void addFont(const QString &path);

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
    qDebug() << "~Config";
}

Config * Config::instance()
{
    static QMutex m;
    static QScopedPointer<Config> i;
    if( Q_UNLIKELY(!i) ) {
        m.lock();
        if( !i ) {
            i.reset(new Config);
        }
        m.unlock();
    }
    return i.data();
}

int Config::init(QGuiApplication *a, QQmlApplicationEngine *e)
{
    configSelf = this;
    p->e = e;
    // 读配置文件
    p->readSetting();

    // QTranslator 如果为局部变量
    // 则加载无效

    switch(p->language)
    {
    case Config::__cn: {
        if( p->ts->load(":/translation/Cn.qm") ) {
            if( a->installTranslator(p->ts) ) {
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

    // 字库加载
    p->fontFamilies.clear();
    int id = -1;
    qDebug() << "- font awesome -";
    p->addFont(":/resource/font/Font Awesome 6 Brands-Regular-400.otf");
    p->addFont(":/resource/font/Font Awesome 6 Duotone-Solid-900.otf");
    p->addFont(":/resource/font/Font Awesome 6 Pro-Light-300.otf");
    p->addFont(":/resource/font/Font Awesome 6 Pro-Regular-400.otf");
    p->addFont(":/resource/font/Font Awesome 6 Pro-Solid-900.otf");
    p->addFont(":/resource/font/Font Awesome 6 Pro-Thin-100.otf");

//    id = QFontDatabase::addApplicationFont(":/resource/font/Font Awesome 6 Brands-Regular-400.otf");
//    qDebug() << id << QFontDatabase::applicationFontFamilies(id).at(0) << QFontDatabase::applicationFontFamilies(id).size();
//    p->fontFamilies.append(QFontDatabase::applicationFontFamilies(id).at(0));
//    id = QFontDatabase::addApplicationFont(":/resource/font/Font Awesome 6 Duotone-Solid-900.otf");
//    qDebug() << id << QFontDatabase::applicationFontFamilies(id).at(0) << QFontDatabase::applicationFontFamilies(id).size();
//    p->fontFamilies.append(QFontDatabase::applicationFontFamilies(id).at(0));
//    id = QFontDatabase::addApplicationFont(":/resource/font/Font Awesome 6 Pro-Light-300.otf");
//    qDebug() << id << QFontDatabase::applicationFontFamilies(id).at(0) << QFontDatabase::applicationFontFamilies(id).size();
//    p->fontFamilies.append(QFontDatabase::applicationFontFamilies(id).at(0));
//    id = QFontDatabase::addApplicationFont(":/resource/font/Font Awesome 6 Pro-Regular-400.otf");
//    qDebug() << id << QFontDatabase::applicationFontFamilies(id).at(0) << QFontDatabase::applicationFontFamilies(id).size();
//    p->fontFamilies.append(QFontDatabase::applicationFontFamilies(id).at(0));
//    id = QFontDatabase::addApplicationFont(":/resource/font/Font Awesome 6 Pro-Solid-900.otf");
//    qDebug() << id << QFontDatabase::applicationFontFamilies(id).at(0) << QFontDatabase::applicationFontFamilies(id).size();
//    p->fontFamilies.append(QFontDatabase::applicationFontFamilies(id).at(0));
//    id = QFontDatabase::addApplicationFont(":/resource/font/Font Awesome 6 Pro-Thin-100.otf");
//    qDebug() << id << QFontDatabase::applicationFontFamilies(id).at(0) << QFontDatabase::applicationFontFamilies(id).size();
//    p->fontFamilies.append(QFontDatabase::applicationFontFamilies(id).at(0));
    qDebug() << "- font awesome -";

    id = QFontDatabase::addApplicationFont(":/resource/font/SourceHanSansCN-Normal.otf");
    if( id >= 0 ) {
        QFont font = a->font();
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);
        font.setFamily(family);
        qDebug() << "set font family:" << family;
        a->setFont(font);
    }

//    qDebug() << "- font awesome -";
//    for(const auto &font : p->fontFamilies) {
//        qDebug() << font;
//    }
//    qDebug() << "font families size:" << p->fontFamilies.size();
//    qDebug() << "- font awesome -";

    p->screen = QGuiApplication::primaryScreen();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    p->screen->setOrientationUpdateMask(Qt::PortraitOrientation |
                                     Qt::LandscapeOrientation |
                                     Qt::InvertedLandscapeOrientation |
                                     Qt::InvertedPortraitOrientation);
#endif
    // 在安卓端
    // 此信号需要在 AndroidManifest.xml 中
    // 将android:screenOrientation
    // 设置为"unspecified"才能触发
    // 否则不会触发槽函数
    QObject::connect(p->screen, static_cast<void (QScreen::*)(Qt::ScreenOrientation)>(&QScreen::orientationChanged),
                     [=](Qt::ScreenOrientation orientation){
        QRect r = p->screen->availableGeometry();
        if( orientation == Qt::LandscapeOrientation
                || orientation == Qt::InvertedPortraitOrientation ) {
            p->isLandscape = true;
        }
        else {
            p->isLandscape = false;
        }
        qDebug() << "orientationChanged screen:" << orientation << r << p->isLandscape;
        emit orientationChanged();
    });

    QObject::connect(a, static_cast<void (QGuiApplication::*)(Qt::ApplicationState)>(&QGuiApplication::applicationStateChanged),
                     this, [=](Qt::ApplicationState state){
        qDebug() << "app state changed:" << state;
#ifndef Q_OS_WIN32
        switch(state) {
        case Qt::ApplicationActive: {
            if( !p->tcpCamera.isNull() ) {
                if( !p->tcpCamera->isOpen() ) {
                    p->tcpCamera->open();
                }
            }
#ifdef Q_OS_IOS
#elif defined (Q_OS_ANDROID)
#endif
        }
            break;
        case Qt::ApplicationSuspended: {
            if( !p->tcpCamera.isNull() ) {
                if( p->tcpCamera->isOpen() ) {
                    p->tcpCamera->close();
                }
            }
        }
            break;
        case Qt::ApplicationInactive: {

        }
            break;
        default: break;
        }
#else
//        switch (state) {
//        case Qt::ApplicationActive: {
//            if( !p->tcpCamera.isNull() ) {
//                if( !p->tcpCamera->isOpen() ) {
//                    p->tcpCamera->open();
//                }
//            }
//        }
//            break;
//        default: {
//            if( !p->tcpCamera.isNull() ) {
//                if( p->tcpCamera->isOpen() ) {
//                    p->tcpCamera->close();
//                }
//            }
//        }
//            break;
//        }
#endif
    }, Qt::QueuedConnection);

    QRect r = p->screen->availableGeometry();
#ifndef Q_OS_WIN32
    p->width = r.width();
    p->height = r.height();
#else
//    p->width = 960;
//    p->height = 515;
    p->width = 515;
    p->height = 960;
#endif
    if( p->width > p->height ) {
        p->isLandscape = true;
    }
    else {
        p->isLandscape = false;
    }
    qDebug() << QThread::currentThreadId() << "windows geometry:" << r << Qt::endl
             <<  "is landscape:" << p->isLandscape;

    QQmlContext *cnt = e->rootContext();


#if defined(Q_OS_IOS)
    p->iosInterface.reset(new IOSInterface);
    cnt->setContextProperty("PhoneApi", p->iosInterface.data());
    p->iosInterface->keepScreenOn();
    SafeArea s;
    p->iosInterface->safeArea(&s);
    p->leftMargin = s.left;
    p->rightMargin = s.right;
    p->topMargin = s.top;
    p->bottomMargin = s.bottom;
#elif defined (Q_OS_ANDROID)
    // 安卓模块
    p->androidInterface.reset(new AndroidInterface);
    cnt->setContextProperty("PhoneApi", p->androidInterface.data());
    p->androidInterface->requestPhotoWritePermission();
    QObject::connect(p->androidInterface.data(), &AndroidInterface::cameraConnect,
                     [=](const int &fd, const QString &deviceName){
        qDebug() << "usb camera:" << deviceName << ", fd:" << fd;
        p->tcpCamera->openUsbCamera(fd);
    });
    QObject::connect(p->androidInterface.data(), &AndroidInterface::cameraDisconnect,
                     [=](const int &fd){
        p->tcpCamera->closeUsbCamera();
    });

    qreal ratio = p->screen->devicePixelRatio();
    p->leftMargin = p->androidInterface->safeAeraLeft() / ratio;
    p->rightMargin = p->androidInterface->safeAeraRight() / ratio;
    p->topMargin = p->androidInterface->safeAreaTop() / ratio;
    p->bottomMargin = p->androidInterface->safeAeraBottom() / ratio;
#else
    p->leftMargin = 0;
    p->rightMargin = 0;
    p->topMargin = 45;
    p->bottomMargin = 45;
#endif

    qDebug() << QString("- safe area -") << Qt::endl
             << "   top     :" << p->topMargin << Qt::endl
             << "   left    :" << p->leftMargin << Qt::endl
             << "   bottom  :" << p->bottomMargin << Qt::endl
             << "   right   :" << p->rightMargin << Qt::endl;

    // 配置模块
    cnt->setContextProperty("Config", this);

    // ffmpeg init
    VideoProcess::initialize();

    // 视频播放器模块
    // 如果ffmpeg没有注册, 调用API会返回-1094995529
    p->videoPlayer.reset(new VideoPlayer);
    p->videoPlayer->setFrameUrl(e, "VideoPlayer");
    cnt->setContextProperty("VideoPlayer", p->videoPlayer.data());

    // 图片文件路径模块
    p->imageModel.reset(new ImageListModel);
    p->imageModel->search(p->imageGalleryPath, e);
    cnt->setContextProperty("ImageModel", p->imageModel.data());

    // 摄像头模块
    p->tcpCamera.reset(new TcpCamera);
    p->tcpCamera->setFrameUrl(e, "TcpCamera");
    cnt->setContextProperty("TcpCamera", p->tcpCamera.data());

    QObject::connect(p->tcpCamera.data(), static_cast<void (TcpCamera::*)(const QString &)>(&TcpCamera::captureFinished),
                     p->imageModel.data(), &ImageListModel::add);
    p->tcpCamera->open();
//    qmlRegisterType<ImagePaintView>("Custom.ImagePaintView", 1, 1,"ImagePaintView");

    return 1;
}

void Config::started()
{
#ifdef Q_OS_ANDROID
    p->androidInterface->openService();
#endif
}

QString Config::documentsPath()
{
    return p->documentsPath;
}

QString Config::imageGalleryPath()
{
    return p->imageGalleryPath;
}

QVariant Config::readSetting(const QString &key, const QVariant &normal)
{
    if( p->setting ) {
        return p->setting->value(key, normal);
    }
    return QVariant();
}

void Config::insertSetting(const QVector<SettingParam> &param)
{
    if( p->setting ) {
        for(const auto &i : param) {
            p->setting->setValue(i.key, i.value);
        }
    }
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
#ifndef Q_OS_WIN32
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

int Config::language()
{
    return p->language;
}

void Config::setLanguage(const int &language)
{
    if( p->language != language ) {
        qDebug() << "set language:" << language;
        p->language = language;
        emit languageChanged();
        qApp->exit(REBOOT_CODE);
    }
}

bool Config::isLandscape()
{
    return p->isLandscape;
}

int Config::leftMargin()
{
    return p->leftMargin;
}

int Config::rightMargin()
{
    return p->rightMargin;
}

int Config::topMargin()
{
    return p->topMargin;
}

int Config::bottomMargin()
{
    return p->bottomMargin;
}

QString Config::fontLight()
{
    return p->fontFamilies.at(2);
}

QString Config::fontSolid()
{
    return p->fontFamilies.at(4);
}

QString Config::fontRegular()
{
    return p->fontFamilies.at(3);
}

QString Config::fontThin()
{
    return p->fontFamilies.at(5);
}

QString Config::fontBrandsRegular()
{
    return p->fontFamilies.at(0);
}

QString Config::fontDuotoneSolid()
{
    return p->fontFamilies.at(1);
}

bool Config::canReadTemperature()
{
#ifdef TEMPERATURE_SDK
    return true;
#else
    return false;
#endif
}

bool Config::isMobile()
{
#ifndef Q_OS_WINDOWS
    return true;
#else
    return false;
#endif
}

void Config::saveSetting()
{
    p->tcpCamera->saveSetting();
    p->saveSetting();
}

void Config::appendLog(const QString &str)
{
    qDebug() << str;
    emit updateLog(str);
}

QString Config::version()
{
    return PROGRAM_VERSION;
}

ConfigPrivate::ConfigPrivate(Config *parent)
{
    f = parent;

    ts = new QTranslator(f);

    oldRotation = 0;
#ifndef Q_OS_WIN32
    accelerometer_x = 0.0;
    accelerometer_y = 0.0;
    accelerometer_z = 0.98;
    accelerometer = new QAccelerometer;
    QObject::connect(accelerometer, &QAccelerometer::readingChanged, [=](){
        QAccelerometerReading *r = accelerometer->reading();
//        accelerometer_x = r->x();
//        accelerometer_y = r->y();
//        accelerometer_z = r->z();
        qreal x = r->x();
        qreal y = r->y();
        int rotation = oldRotation;
        if (x > -2.5 && x <= 2.5 && y > 7.5 && y <= 10
                /*&& (rotation != 270 && rotation != -90)*/
                && (rotation != 0 && rotation != -360)) {
            // home button bottom
            if( rotation == -270 ) {
                rotation = -360;
            }
            else {
                rotation = 0;
            }
        }
        else if (x > 7.5 && x <= 10 && y > -2.5 && y <= 2.5
                 /*&& (rotation != 0 && rotation != -360)*/
                 && (rotation != -270 && rotation != 90)) {
            // home button right
            if( rotation == -180 || rotation == -360 ) {
                rotation = -270;
            }
            else {
                rotation = rotation == -90 ? -270 : 90;
            }
        }
        else if (x > -2.5 && x <= 2.5 && y > -10 && y <= -7.5
                 /*&& (rotation != 90 && rotation != -270)*/
                 && (rotation != 180 && rotation != -180)) {
            // home button top
            if( rotation == -90 || rotation == -270 ) {
                rotation = -180;
            }
            else {
                rotation = 180;
            }
        }
        else if (x > -10 && x <= -7.5 && y > -2.5 && y < 2.5
                 /*&& (rotation != 180 && rotation != -180)*/
                 && (rotation != -90 && rotation != 270)) {
            // home button left
            if( rotation == 0 || rotation == -180 ) {
                rotation = -90;
            }
            else {
                rotation = rotation == -270 ? -90 : 270;
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
    });

    old_ts = QDateTime::currentDateTime().toMSecsSinceEpoch();
    accelerometer->start();
//    gyroscope->start();

#endif
}

ConfigPrivate::~ConfigPrivate()
{
#ifndef Q_OS_WIN32
    accelerometer->stop();
    gyroscope->stop();
    accelerometer->deleteLater();
    gyroscope->deleteLater();
#endif

    tcpCamera->saveSetting();
    saveSetting();
    setting->sync();
    delete setting;
    setting = NULL;
}

void ConfigPrivate::readSetting()
{
#ifndef Q_OS_WIN32
    documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QString("/");
#else
    documentsPath = QGuiApplication::applicationDirPath() + QString("/");
#endif
    QString path = documentsPath + QString("setting.ini");

    qDebug() << "read setting path:" << path << ", file exists:" << QFileInfo::exists(path);
    setting = new QSettings(path, QSettings::IniFormat);

    language = setting->value("Normal/Language", 0).toInt();
    imageGalleryPath = documentsPath + QString("Hotimage/");
    if( !QFileInfo::exists(imageGalleryPath) ) {
        QDir dir;
        dir.mkdir(imageGalleryPath);

        qDebug() << "image gallery path exists:" << QFileInfo::exists(imageGalleryPath)
                 << ", image gallery path:" << imageGalleryPath;
    }
}

void ConfigPrivate::saveSetting()
{
    qDebug() << "save setting path:" << setting->fileName();
    setting->setValue("Normal/Language", language);
}

void ConfigPrivate::addFont(const QString &path)
{
    QFile file(path);
    if( !file.open(QIODevice::ReadOnly) ) {
        qDebug() << "ERROR: open font file fail:" << path;
        return;
    }

    int id = QFontDatabase::addApplicationFontFromData(file.readAll());
    qDebug() << QFontDatabase::applicationFontFamilies(id).at(0);
    fontFamilies.append(QFontDatabase::applicationFontFamilies(id).at(0));
    file.close();
}

Config * Config::configSelf = nullptr;
