#include "tcpcamera.h"

#ifdef Q_OS_ANDROID
#include "xtherm/thermometry.h"
#endif

#include "Config/config.h"
#include "VideoProcess/videoprocess.h"
#include "Wireless/wireless.h"


#include "AndroidInterface/androidinterface.h"

#include "QGuiApplication"

#include "QDir"
#include "QStandardPaths"
#include "QFileInfo"
#include "QDebug"
#include "QThread"
#include "QDateTime"
#include "QGuiApplication"
#include "QPainter"
#include "QSettings"
#include "QMutex"
#include "QNetworkInterface"
#include "QTimer"

#include "QtNetwork/qnetworkaccessmanager.h"
//#include "QNetworkConfigurationManager"
//#include "QNetworkConfiguration"

#include "thread"

#define IMAGE_Y_OFFSET          4

class TcpCameraPrivate : public QObject
{
    Q_OBJECT
public:
    explicit TcpCameraPrivate(TcpCamera *parent = nullptr);
    ~TcpCameraPrivate();

    VideoProcess *encode;

//    QImage image;
    ImageProvider *provider;

    tcp_config cfg;

    HandShake handshake;

    QThread *thread;
    QTimer *timer;
    QTcpSocket *socket;
    QByteArray buf;

    int frameSize;

    bool hasFrame;

//    time_t timer0;
//    time_t timer1;
    int exit;
    double fps;

    qint64 lastTimeStamp;
    int fpsCount;

    uint32_t frameCount;

#ifdef TEMPERATURE_SDK
    float temperatureTable[16384];
    float *temperatureData;
#endif

    std::thread captureThread;

    void readSetting();
    void saveSetting();
    void capture();
    void shutter();
    void setPalette(const int &index);
    void setCameraParam(const qreal &emiss, const qreal &reflected,
                        const qreal &ambient, const qreal &humidness,
                        const qreal &correction, const int &distance);
    bool isValidIpv4Addres(const QString &ip);

    QMutex unpackMutex;

    int searching;
    void searchDevice(const QString &devIp = QString(""));

    bool showTemp;

    QString localIP;
    QString deviceIP;

    int keyValue;
    int oldKeyValue;
    QTimer *longTimer;
    QTimer *shortTimer;
    void keyPressed(const int &key);

    QString cameraSN;

private:
    TcpCamera *f;

Q_SIGNALS:
    void write(const QByteArray &byte);
    void research(const QString &devIp);

private Q_SLOTS:
    void onReadyRead();

};

TcpCamera::TcpCamera(QObject *parent)
    : QObject(parent)
    , p(new TcpCameraPrivate(this))
{
}

TcpCamera::~TcpCamera()
{
    close();
}

TcpCamera * TcpCamera::instance()
{
    static QMutex m;
    static QScopedPointer<TcpCamera> i;
    if( Q_UNLIKELY(!i) ) {
        m.lock();
        if( !i ) {
            i.reset(new TcpCamera);
        }
        m.unlock();
    }
    return i.data();
}

bool TcpCamera::isOpen()
{
    return p->thread->isRunning();
}

bool TcpCamera::isConnected()
{
    return isOpen() ? p->hasFrame : false;
}

void TcpCamera::open()
{
    if( isOpen() ) {
        return;
    }
    p->frameCount = 0;
    p->lastTimeStamp = 0;
    p->fpsCount = 0;
    p->thread->start();
}

void TcpCamera::close()
{
    p->exit = 1;
    if( isOpen() ) {
        if( encoding() ) {
            closeRecord();
        }
        p->thread->quit();
        p->thread->wait();
        qDebug() << "tcp thread close";
    }
}

double TcpCamera::fps()
{
    return p->fps;
}

void TcpCamera::capture()
{
    if( !isConnected() ) {
        return;
    }
    p->capture();
}

ImageProvider * TcpCamera::provider()
{
    return p->provider;
}

QString TcpCamera::videoFrameUrl()
{
    return p->provider->qmlUrl();
}

void TcpCamera::shutter()
{
    p->shutter();
}

void TcpCamera::setPalette(const int &index)
{
    p->setPalette(index);
    emit paletteChanged();
}

void TcpCamera::setCameraParam(const qreal &emiss, const qreal &reflected,
                               const qreal &ambient, const qreal &humidness,
                               const qreal &correction, const int &distance)
{
    p->setCameraParam(emiss, reflected, ambient, humidness, correction, distance);
}

int TcpCamera::palette()
{
    return p->cfg.set.palette;
}

qreal TcpCamera::emiss()
{
    return p->cfg.set.emiss;
}

qreal TcpCamera::reflected()
{
    return p->cfg.set.reflected;
}

qreal TcpCamera::ambient()
{
    return p->cfg.set.ambient;
}

qreal TcpCamera::humidness()
{
    return p->cfg.set.humidness;
}

qreal TcpCamera::correction()
{
    return p->cfg.set.correction;
}

uint16_t TcpCamera::distance()
{
    return p->cfg.set.distance;
}

bool TcpCamera::showTemp()
{
    return p->showTemp;
}

void TcpCamera::setShowTemp(const bool &show)
{
    p->showTemp = show;
    emit showTempChanged();
}

bool TcpCamera::encoding()
{
    return p->encode->status();
}

void TcpCamera::openRecord()
{
    if( isConnected() )
    {
        emit recordTimeChanged();
        EncodeConfig cfg;
        QString fileName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + QString(".avi");
        QString path = g_Config->imageGalleryPath();
        path.append("REC_" + fileName);

        cfg.filePath = path;
        cfg.width = p->cfg.header.width;
        cfg.height = p->cfg.header.height - IMAGE_Y_OFFSET;
        cfg.fps = 25;
        cfg.encodePixel = p->encode->pixel(QImage::Format_RGB888);
        p->encode->openEncode(cfg);
        emit encodingChanged();
    }
}

void TcpCamera::closeRecord()
{
    QString path = p->encode->filePath();
    p->encode->closeEncode();
    //  2021-12-24
    //  此信号会调用capture thread的join()
    //  未处理好导致以为是closeEncode()里thread join()报错
    emit captureFinished(path);
    emit msg(tr("Save record path:") + path);
    emit encodingChanged();
}

QString TcpCamera::recordTime()
{
    return p->encode->recordTime();
}

QString TcpCamera::cameraSN()
{
    return p->cameraSN;
}

QString TcpCamera::localIp()
{
    return p->localIP;
}

QString TcpCamera::deviceIp()
{
    return p->deviceIP;
}

QString TcpCamera::hotspotSSID()
{
    return QString::fromStdString(p->cfg.set.hotspot_ssid);
}

QString TcpCamera::hotspotPassword()
{
    return QString::fromStdString(p->cfg.set.hotspot_password);
}

bool TcpCamera::setWirelessParam(const QString &deviceIp, const QString &ssid, const QString &password)
{
    if( !p->isValidIpv4Addres(deviceIp) ) {
        return false;
    }

    p->cfg.set.hotspot_ssid = ssid.toStdString();
    p->cfg.set.hotspot_password = password.toStdString();

    if( isConnected() ) {
        p->deviceIP = deviceIp;
        if( !ssid.isEmpty() && !password.isEmpty() ) {
            p->cfg.set.type = HandShake::__hotspot_info;
            QByteArray byte = p->handshake.pack(p->cfg.set);
            emit p->write(byte);
        }
    }
    else {
        p->searching = -1;
        emit p->research(deviceIp);
    }
    emit wirelessParamChanged();
    return true;
}

TcpCameraPrivate::TcpCameraPrivate(TcpCamera *parent)
    : QObject(nullptr)
{
    readSetting();

    exit = 1;
    f = parent;
    hasFrame = false;
    QObject::connect(f, &TcpCamera::captureFinished, f, [=](){
        if( captureThread.joinable() ) {
            captureThread.join();
        }
    }, Qt::QueuedConnection);

//    image = QImage();
    provider = new ImageProvider("tcpcamera");
#ifdef TEMPERATURE_SDK
    temperatureData = NULL;
#endif
    fps = 0.0;

    keyValue = 1;
    oldKeyValue = 1;
    longTimer = new QTimer(this);
    longTimer->setInterval(3000);
    longTimer->setSingleShot(true);
    QObject::connect(longTimer, &QTimer::timeout, [=](){
        qDebug() << "key hold:" << keyValue;
        if( shortTimer->isActive() ) {
            shortTimer->stop();
        }
        switch (keyValue) {
        case 0: {
            f->openRecord();
        }
            break;
        default: break;
        }
        keyValue = -1;
    });
    shortTimer = new QTimer(this);
    shortTimer->setInterval(150);
    shortTimer->setSingleShot(true);
    QObject::connect(shortTimer, &QTimer::timeout, [=](){
        qDebug() << "key pressed" << keyValue;
        if( longTimer->isActive() ) {
            longTimer->stop();
        }
        switch (keyValue) {
        case 0: {
            capture();
        }
            break;
        default: break;
        }
        keyValue = -1;
    });

    thread = new QThread;
    QObject::connect(thread, &QThread::started, [=](){
        exit = 0;
        buf.clear();
        timer = new QTimer;
        timer->setInterval(5000);
        timer->setSingleShot(true);

        cameraSN = QString("");

        socket = new QTcpSocket;
        QObject::connect(socket, &QTcpSocket::readyRead, this, &TcpCameraPrivate::onReadyRead, Qt::QueuedConnection);
        QObject::connect(socket, &QTcpSocket::stateChanged,
                         this, [=](QAbstractSocket::SocketState state){
            if( socket == nullptr ) {
                return ;
            }

            if( state == QAbstractSocket::ConnectedState ) {
                qDebug() << "first request frame pack" << timer << socket << exit;
                timer->start();
                cfg.set.type = HandShake::__handshake;
                QByteArray byte = handshake.pack(cfg.set);
                emit write(byte);
            }
        }, Qt::QueuedConnection);
        QObject::connect(socket, &QTcpSocket::disconnected, this, [=](){
            hasFrame = false;
            cameraSN = QString("");
            if( socket != nullptr ) {
                qDebug() << "socket disconnect:" << socket->state();
                emit f->connectStatusChanged();
                unpackMutex.lock();
                buf.clear();
                unpackMutex.unlock();

                // 此信号有可能为上次接收, 此时socket已重新连接
                if( socket->state() != QTcpSocket::ConnectedState ) {
                    searching = 1;
                    searchDevice();
                }
            }
        }, Qt::QueuedConnection);

        QObject::connect(this, static_cast<void (TcpCameraPrivate::*)(const QByteArray &)>(&TcpCameraPrivate::write),
                         this, [=](const QByteArray &byte){
            if( socket == nullptr ) {
                return ;
            }
            if( socket->state() == QTcpSocket::ConnectedState ) {
                int size = socket->write(byte.data(), byte.size());
                socket->waitForBytesWritten();
                qDebug() << "socket write:" << size;
            }
        }, Qt::QueuedConnection);

        QObject::connect(this, static_cast<void (TcpCameraPrivate::*)(const QString &)>(&TcpCameraPrivate::research),
                         this, [=](const QString &devIp){
            if( socket == nullptr ) {
                return ;
            }

            while ((searching == -1) && (exit != 1)) {
                QThread::msleep(15);
            }
            qDebug() << "research device ip:" << devIp;
            if( socket->state() != QTcpSocket::ConnectedState ) {
                deviceIP = devIp;
                searching = 1;
                searchDevice();
            }
            else {
                qDebug() << "research fail, socket connected";
            }
        }, Qt::QueuedConnection);

        QObject::connect(timer, &QTimer::timeout, [=](){
            if( socket == nullptr ) {
                return ;
            }

            timer->start();
            cfg.set.type = HandShake::__handshake;
            QByteArray byte = handshake.pack(cfg.set);
            emit write(byte);
        });

        qDebug() << QThread::currentThreadId() << "socket start connect";

        searching = 1;
#ifdef Q_OS_WIN
        searchDevice(SERVER_IP);
#else
        searchDevice();
#endif
    });

    QObject::connect(thread, &QThread::finished, [=](){
        if( socket->state() == QTcpSocket::ConnectedState ) {
            cfg.set.type = HandShake::__disconnect;
            QByteArray byte = handshake.pack(cfg.set);
            socket->write(byte);
            bool flag = socket->waitForBytesWritten();
            qDebug() << "send disconnect:" << flag;
        }

        searching = 0;

        timer->stop();
        timer->deleteLater();
        timer = nullptr;

        socket->deleteLater();
        socket = nullptr;

//        image = QImage();
//        provider->clear();
        provider->release();

        if( f->encoding() ) {
            f->closeRecord();
        }

        emit f->videoFrameChanged();
//        emit f->videoFrame(QImage());

#ifdef TEMPERATURE_SDK
        if( temperatureData ) {
            free(temperatureData);
            temperatureData = NULL;
        }
#endif

        buf.clear();

        emit f->connectStatusChanged();
        qDebug() << QThread::currentThreadId() << "socket release";
    });

    this->moveToThread(thread);

    encode = new VideoProcess;
    QObject::connect(encode, &VideoProcess::recordTimeChanged, f, &TcpCamera::recordTimeChanged);
    QObject::connect(encode, &VideoProcess::error, f, [=](){
        encode->closeEncode();
        qDebug() << QString::fromStdString(encode->lastError());
        emit f->msg(tr("Record video fail"));
        emit f->encodingChanged();
    }, Qt::QueuedConnection);
    QObject::connect(encode, &VideoProcess::statusChanged, f, &TcpCamera::encodingChanged);
}

TcpCameraPrivate::~TcpCameraPrivate()
{
//#ifdef Q_OS_ANDROID
    encode->closeEncode();
    encode->deleteLater();
//#endif
    saveSetting();
}

void TcpCameraPrivate::searchDevice(const QString &devIp)
{
    hasFrame = false;
    // 初次尝试连接上次已连接的设备IP
    if( !deviceIP.isEmpty() ) {
        socket->connectToHost(deviceIP, cfg.port);
        socket->waitForConnected(1000);
        if( socket->state() == QAbstractSocket::ConnectedState ) {
            const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
            for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
                if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
                     localIP = address.toString();
                }
            }

            qDebug() << "connect prev success, device ip:" << deviceIP << ", local ip:" << localIP;
            emit f->wirelessParamChanged();
            return;
        }
    }

    while ((exit != 1) && (searching == 1))
    {
        localIP.clear();
        deviceIP.clear();
        const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
        for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
                 localIP = address.toString();
            }
        }

        QString searchIp = devIp.isEmpty() ? localIP : devIp;

        qDebug() << "local ip:" << localIP << ", search ip:" << searchIp;
        if( !searchIp.isNull() )
        {
            QString ip_front = searchIp.left(searchIp.lastIndexOf('.') + 1);
            int index = 1;
            while ((searching == 1) && socket && (exit != 1))
            {
                QString ip = ip_front + QString::number(index);
                if( ip == localIP ) {
                    index ++;
                    continue;
                }

//                qDebug() << "connect ip:" << ip << "port:" << cfg.port << "local:" << localIP << "search ip:" << searchIp;
                socket->connectToHost(ip, cfg.port);
                socket->waitForConnected(50);
                if( socket->state() == QAbstractSocket::ConnectedState ) {
                    deviceIP = ip;
                    emit f->wirelessParamChanged();
                    break;
                }
                index ++;
                if( index > 255 ) {
                    qDebug() << "resreach, ip:" << ip_front;
                    index = 1;
                }
            }
        }
        else {
            QThread::msleep(1500);
        }

        searching = 0;
        if( !deviceIP.isNull() ) {
            qDebug() << "connect success, device ip:" << deviceIP;
            break;
        }
    }
}

void TcpCameraPrivate::keyPressed(const int &key)
{
    if( longTimer->isActive() ) {
        longTimer->stop();
    }
    else {
        return;
    }
    switch (key) {
    case 0: {
        capture();
    }
        break;
    default: break;
    }
}

void TcpCameraPrivate::onReadyRead()
{
    unpackMutex.lock();
    buf.push_back(socket->readAll());
    unpackMutex.unlock();

    int headerSize = sizeof (t_header);
    if( buf.size() < headerSize ) {
        return;
    }

    memcpy(&cfg.header, buf.data(), headerSize);
    int totalSize = cfg.header.bufferLength + headerSize;
    if( buf.size() >= totalSize )
    {
        if( !cfg.header.isValidHeader() )
        {
            // 包丢失
            // 重新校验包
            qDebug() << "invalid pack, get buf size:" << buf.size() << ", total size:" << totalSize;

            int i = 0;
            int flag = 0;

            int size = buf.size();
            while (i < size)
            {
                char first = buf.at(i);
                if( first == 'T' )
                {
                    QByteArray head = buf.mid(i, headerSize);
                    if( cfg.header.isValidHeader(head.data()) )
                    {
                        qDebug() << "remove invalid data, size:" << i;
                        unpackMutex.lock();
                        buf.remove(0, i);
                        unpackMutex.unlock();
                        flag = 1;
                        break;
                    }
                    else {
                        i ++;
                    }
                }
                else {
                    i ++;
                }
            }

            if( flag == 0 ) {
                // 当前数据无效
                qDebug() << "pack invalid:" << size;
                unpackMutex.lock();
                buf.remove(0, size);
                unpackMutex.unlock();
            }
        }
        else
        {
#ifdef TEMPERATURE_SDK
            if( temperatureData == NULL ) {
                temperatureData = (float*)calloc(cfg.header.width * (cfg.header.height - IMAGE_Y_OFFSET) + 10, sizeof(float));
            }
#endif

            frameCount ++;
            fpsCount ++;
            if( lastTimeStamp == 0 ) {
                lastTimeStamp = QDateTime::currentMSecsSinceEpoch();
            }
            else {
                qint64 curTimeStamp = QDateTime::currentMSecsSinceEpoch();
                if( (curTimeStamp - lastTimeStamp) >= 1000 ) {
                    lastTimeStamp = 0;
                    fps = fpsCount;
                    fpsCount = 0;
                }
            }

            uint16_t *data = reinterpret_cast<uint16_t *>(buf.data() + headerSize);
            uint16_t *temp = data + (cfg.header.width * (cfg.header.height - IMAGE_Y_OFFSET));
            float floatFpaTmp;
            float correction;
            float Refltmp;
            float Airtmp;
            float humi;
            float emiss;
            unsigned short distance;
            char sn[32];//camera序列码
            char cameraSoftVersion[16];//camera软件版本
            unsigned short shutTemper;
            float floatShutTemper;//快门温度
            unsigned short coreTemper;
            float floatCoreTemper;//外壳温度

            int amountPixels=0;
            switch (cfg.header.width)
            {
            case 384:
                amountPixels=cfg.header.width*(4-1);
                break;
            case 240:
                amountPixels=cfg.header.width*(4-3);
                break;
            case 256:
                amountPixels=cfg.header.width*(4-3);
                break;
            case 640:
                amountPixels=cfg.header.width*(4-1);
                break;
            }

            memcpy(&shutTemper,temp+amountPixels+1,sizeof(unsigned short));
            floatShutTemper=shutTemper/10.0f-273.15f;//快门温度
            memcpy(&coreTemper,temp+amountPixels+2,sizeof(unsigned short));//外壳
            floatCoreTemper=coreTemper/10.0f-273.15f;
            memcpy((unsigned short*)cameraSoftVersion,temp+amountPixels+24,16*sizeof(uint8_t));//camera soft version
            memcpy((unsigned short*)sn,temp+amountPixels+32,32*sizeof(uint8_t));//SN

            int userArea=amountPixels+127;
            memcpy(&correction,temp+userArea,sizeof( float));//修正
            userArea=userArea+2;
            memcpy(&Refltmp,temp+userArea,sizeof( float));//反射温度
            userArea=userArea+2;
            memcpy(&Airtmp,temp+userArea,sizeof( float));//环境温度
            userArea=userArea+2;
            memcpy(&humi,temp+userArea,sizeof( float));//湿度
            userArea=userArea+2;
            memcpy(&emiss,temp+userArea,sizeof( float));//发射率
            userArea=userArea+2;
            memcpy(&distance,temp+userArea,sizeof(unsigned short));//距离

#ifdef TEMPERATURE_SDK
            if( (frameCount % 4500) == 25 ) {
                thermometryT4Line(cfg.header.width,
                                  cfg.header.height,
                                  temperatureTable,
                                  temp,
                                  &floatFpaTmp,
                                  &correction,
                                  &Refltmp,
                                  &Airtmp,
                                  &humi,
                                  &emiss,
                                  &distance,
                                  CAMERA_LEN,
                                  SHUTTER_FIX,
                                  RANGE_MODE);
                if( frameCount > 9000 ) {
                    frameCount = 0;
                }
            }

            thermometrySearch(cfg.header.width,
                              cfg.header.height,
                              temperatureTable,
                              data,
                              temperatureData,
                              RANGE_MODE,
                              OUTPUT_MODE);
#endif
            if( !QString(sn).isEmpty() && cameraSN.isEmpty() ) {
                cameraSN = QString(sn);
                emit f->cameraSNChanged();
            }

    //        timer0 = timer1;
    //        timer1 = clock();
    //        uint8_t *bit = provider->data();

            QImage *image = provider->image(cfg.header.width, cfg.header.height - IMAGE_Y_OFFSET);
            uint8_t *bit = image->bits();


    //        fps = CLOCKS_PER_SEC / (double)(timer1 - timer0);

            if( cfg.header.pixelFormat == __yuyv ) {
                convert().yuv422_to_rgb(reinterpret_cast<uint8_t *>(data), bit,
                                        cfg.header.width, cfg.header.height - IMAGE_Y_OFFSET);
            }
            else if( cfg.header.pixelFormat == __yuv420 ) {
                convert().yuv420p_to_rgb(reinterpret_cast<uint8_t *>(data), bit,
                                         cfg.header.width, cfg.header.height - IMAGE_Y_OFFSET);
            }

            if( !hasFrame ) {
                hasFrame = true;
                emit f->paletteChanged();
                emit f->cameraParamChanged();
                emit f->connectStatusChanged();
            }

            unpackMutex.lock();
            buf.remove(0, totalSize);
            unpackMutex.unlock();

#ifdef TEMPERATURE_SDK
            // 画温度信息
            if( showTemp ) {
                QPainter pr(image);
                QFontMetrics metrics(pr.font());

                // 最大温度
                pr.setPen(Qt::red);
                pr.drawLine(temperatureData[1] - 10, temperatureData[2],
                            temperatureData[1] + 10, temperatureData[2]);
                pr.drawLine(temperatureData[1], temperatureData[2] - 10,
                            temperatureData[1], temperatureData[2] + 10);

                QString str = QString::number(temperatureData[3], 'f', 1);
                int fontW = metrics.horizontalAdvance(str);
                int fontH = metrics.height();
                pr.drawText(temperatureData[1], temperatureData[2] - fontH,
                            fontW, fontH,
                            Qt::AlignCenter,
                            str);

                // 最小温度
                pr.setPen(Qt::blue);
                pr.drawLine(temperatureData[4] - 10, temperatureData[5],
                            temperatureData[4] + 10, temperatureData[5]);
                pr.drawLine(temperatureData[4], temperatureData[5] - 10,
                            temperatureData[4], temperatureData[5] + 10);

                str = QString::number(temperatureData[6], 'f', 1);
                fontW = metrics.horizontalAdvance(str);
                fontH = metrics.height();
                pr.drawText(temperatureData[4], temperatureData[5] - fontH,
                            fontW, fontH,
                            Qt::AlignCenter,
                            str);

                // 中心温度
                pr.setPen(Qt::white);
                int cx = cfg.header.width / 2;
                int cy = cfg.header.height / 2;
                pr.drawLine(cx - 10, cy, cx + 10, cy);
                pr.drawLine(cx, cy - 10, cx, cy + 10);

                str = QString::number(temperatureData[0], 'f', 1);
                fontW = metrics.horizontalAdvance(str);
                fontH = metrics.height();
                pr.drawText(cx, cy - fontH,
                            fontW, fontH,
                            Qt::AlignCenter,
                            str);

                // 帧率
                pr.setPen(Qt::white);
                str = QString::number(fps, 'f', 2);
                fontW = metrics.horizontalAdvance(str);
                fontH = metrics.height();
                pr.drawText(10, 10,
                            fontW, fontH,
                            Qt::AlignCenter,
                            str);
            }
#endif

            if( exit == 0 ) {
                timer->start();
                cfg.set.type = HandShake::__handshake;
                QByteArray byte = handshake.pack(cfg.set);
                socket->write(byte);
            }

            if( encode->status() ) {
                encode->pushEncode(image->copy());
            }


            emit f->videoFrameChanged();
        }
    }
}

void TcpCameraPrivate::readSetting()
{
    // 初始化 参数
    deviceIP.clear();
    localIP.clear();
    searching = 0;


#ifndef Q_OS_WIN32
    QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QString("/cameraparam.ini");
#else
    QString path = QGuiApplication::applicationDirPath() + QString("/cameraparam.ini");
#endif
    QSettings *s = new QSettings(path, QSettings::IniFormat);

    deviceIP = s->value("Socket/DeviceIP", "").toString();

    cfg.ip = SERVER_IP;
    cfg.port = SERVER_PORT;
    cfg.set.palette = s->value("Normal/palette", 0).toUInt();
    cfg.set.mode = s->value("Normal/mode", CAM_OUTPUT_YUYV_MODE).toUInt();
    cfg.set.emiss = s->value("Normal/emiss", 1.0).toDouble();
    cfg.set.reflected = s->value("Normal/reflected", 1.0).toDouble();
    cfg.set.ambient = s->value("Normal/ambient", 1.0).toDouble();
    cfg.set.humidness = s->value("Normal/humidness", 1.0).toDouble();
    cfg.set.correction = s->value("Normal/correction", 1.0).toDouble();
    cfg.set.distance = s->value("Normal/distance", 0).toUInt();

    showTemp = s->value("Normal/showtemp", false).toBool();

    cfg.set.hotspot_ssid = s->value("Hotspot/ssid", "").toString().toStdString();
    cfg.set.hotspot_password = s->value("Hotspot/password", "").toString().toStdString();

    qDebug() << "read setting path:" << path << ", file exists:" << QFileInfo::exists(path);
    qDebug() << "- camera param -" << Qt::endl
             << "      emiss:" << cfg.set.emiss << Qt::endl
             << "  reflected:" << cfg.set.reflected << Qt::endl
             << "    ambient:" << cfg.set.ambient << Qt::endl
             << "  humidness:" << cfg.set.humidness << Qt::endl
             << " correction:" << cfg.set.correction << Qt::endl
             << "   distance:" << cfg.set.distance;
//    emit f->paletteChanged();

    delete s;
    s = NULL;
}

void TcpCameraPrivate::saveSetting()
{
#ifndef Q_OS_WIN32
    QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QString("/cameraparam.ini");
#else
    QString path = QGuiApplication::applicationDirPath() + QString("/cameraparam.ini");
#endif
    qDebug() << "save setting path:" << path;
    QSettings *s = new QSettings(path, QSettings::IniFormat);

    s->setValue("Socket/DeviceIP", deviceIP);

    s->setValue("Normal/palette", cfg.set.palette);
    s->setValue("Normal/mode", cfg.set.mode);
    s->setValue("Normal/emiss", cfg.set.emiss);
    s->setValue("Normal/reflected", cfg.set.reflected);
    s->setValue("Normal/ambient", cfg.set.ambient);
    s->setValue("Normal/humidness", cfg.set.humidness);
    s->setValue("Normal/correction", cfg.set.correction);
    s->setValue("Normal/distance", cfg.set.distance);

    s->setValue("Normal/showtemp", showTemp);

    s->setValue("Hotspot/ssid", QString::fromStdString(cfg.set.hotspot_ssid));
    s->setValue("Hotspot/password", QString::fromStdString(cfg.set.hotspot_password));

    s->sync();
    delete s;
    s = NULL;
}

void TcpCameraPrivate::capture()
{
    if( !captureThread.joinable() ) {
        captureThread = std::thread([=](){
            QPixmap pix = QPixmap::fromImage(provider->image());
            QString fileName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + QString(".jpg");
            QString path = g_Config->imageGalleryPath();

            path.append("IMG_" + fileName);
            if( pix.save(path, "JPG") )
            {
                qDebug() << "capture success:" << path;
                emit f->captureFinished(path);
            }
            else {
                qDebug() << "capture fail:" << path;
                emit f->msg(tr("Captrue fail"));
            }
        });
    }
}

void TcpCameraPrivate::shutter()
{
    cfg.set.type = HandShake::__shuffer;
    QByteArray byte = handshake.pack(cfg.set);
    emit write(byte);
}

void TcpCameraPrivate::setPalette(const int &index)
{
    cfg.set.type = HandShake::__palette;
    cfg.set.palette = index;
    QByteArray byte = handshake.pack(cfg.set);
    emit write(byte);
}

void TcpCameraPrivate::setCameraParam(const qreal &emiss, const qreal &reflected,
                                      const qreal &ambient, const qreal &humidness,
                                      const qreal &correction, const int &distance)
{
    cfg.set.type = HandShake::__config;
    cfg.set.emiss = emiss;
    cfg.set.reflected = reflected;
    cfg.set.ambient = ambient;
    cfg.set.humidness = humidness;
    cfg.set.correction = correction;
    cfg.set.distance = distance;
    QByteArray byte = handshake.pack(cfg.set);
    emit f->cameraParamChanged();
    emit write(byte);
}

bool TcpCameraPrivate::isValidIpv4Addres(const QString &ip)
{
    int start = 0;
    int point = 0;
    for(int i = 0; i < ip.length(); i ++)
    {
        if( ip.at(i) == '.' ) {
            if( (i - start) < 1 ) {
                return false;
            }
            QString str = ip.mid(start, i - start);
            int num = str.toInt();
            if( num < 0 || num > 255 ) {
                return false;
            }
            start = i + 1;
            point ++;
            if( point == 3 )
            {
                if( (ip.length() - start) < 1 ) {
                    return false;
                }
                str = ip.mid(start, ip.length() - start);
                num = str.toInt();
                if( num < 0 || num > 255 ) {
                    return false;
                }
            }
        }
    }
    return true;
}

#include "tcpcamera.moc"
