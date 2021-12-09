#include "tcpcamera.h"

#ifdef Q_OS_ANDROID
#include "xtherm/thermometry.h"

#include "VideoProcess/videoprocess.h"
#endif

#include "AndroidInterface/androidinterface.h"

#include "QGuiApplication"

#include "QDebug"
#include "QThread"
#include "QDateTime"
#include "QGuiApplication"
#include "QPainter"
#include "QFileInfo"
#include "QStandardPaths"
#include "QDir"
#include "QStandardPaths"
#include "QSettings"
#include "QMutex"

#include "thread"

#define IMAGE_Y_OFFSET          4

class TcpCameraPrivate : public QObject
{
    Q_OBJECT
public:
    explicit TcpCameraPrivate(TcpCamera *parent = nullptr);
    ~TcpCameraPrivate();

#ifdef Q_OS_ANDROID
    VideoProcess *encode;
#endif

    ImageProvider *provider;

    tcp_config cfg;

    HandShake handshake;

    QThread *thread;
    QTcpSocket *socket;
    QByteArray buf;

//    QImage image;
    int frameSize;

    time_t timer0;
    time_t timer1;
    bool exit;
    double fps;

    uint32_t frameCount;
    float temperatureTable[16384];
    float *temperatureData;

    std::thread captureThread;

    void readSetting();
    void saveSetting();
    void capture();
    void shutter();
    void setPalette(const int &index);
    void setCameraParam(const qreal &emiss, const qreal &reflected,
                        const qreal &ambient, const qreal &humidness,
                        const qreal &correction, const int &distance);

    int runDecode;
    std::thread unpackThread;
    QMutex unpackMutex;
    void openUnpack();
    void unpack(const int &data_size);
    void closeUnpack();

    bool showTemp;

private:
    TcpCamera *f;


Q_SIGNALS:
    void write(const QByteArray &byte);

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

    if( p->captureThread.joinable() ) {
        p->captureThread.join();
    }
}

bool TcpCamera::isOpen()
{
    return p->thread->isRunning();
}

bool TcpCamera::isConnected()
{
    if( isOpen() ) {
        return (p->socket->state() == QAbstractSocket::ConnectedState);
    }
    return false;
}

void TcpCamera::open()
{
    if( isOpen() || p->cfg.ip.isEmpty() ) {
        return;
    }
    p->frameCount = 0;
    p->thread->start();
}

//void TcpCamera::open(tcp_config *s)
//{
//    if( isOpen() || s->ip.isEmpty() ) {
//        return;
//    }
//    p->cfg.ip = s->ip;
//    p->cfg.port = s->port;
//    memcpy(&p->cfg.set, &s->set, sizeof (t_setting_page));
//    p->frameCount = 0;
//    p->thread->start();
//}

void TcpCamera::close()
{
    if( isOpen() ) {
        p->exit = true;
        p->thread->quit();
        p->thread->wait();
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
#ifdef Q_OS_ANDROID
    return p->encode->status();
#else
    return false;
#endif
}

void TcpCamera::openRecode()
{
    if( isConnected() ) {
#ifdef Q_OS_ANDROID
        if( p->encode->status() ) {
            p->encode->closeEncode();

            emit msg(tr("Save record path:") + p->encode->filePath());
        }
        else {
            emit recordTimeChanged();
            EncodeConfig cfg;
            QString fileName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + QString(".avi");
#ifdef Q_OS_ANDROID
            QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
#else
            QString path = QApplication::applicationDirPath();
#endif
            path.append("/Hotimage");
            if( !QFileInfo::exists(path) ) {
                QDir d;
                d.mkdir(path);
            }

            path.append("/VIDEO_" + fileName);

            cfg.filePath = path;
            cfg.width = p->cfg.cam.w;
            cfg.height = p->cfg.cam.h;
            cfg.fps = 25;
            cfg.encodePixel = p->encode->pixel(QImage::Format_RGB888);
            p->encode->openEncode(cfg);
        }

        emit encodingChanged();
#endif
    }
}

QString TcpCamera::recordTime()
{
#ifdef Q_OS_ANDROID
    return p->encode->recordTime();
#else
    return QString("00:00");
#endif
}

TcpCameraPrivate::TcpCameraPrivate(TcpCamera *parent)
    : QObject(nullptr)
{
    readSetting();

    f = parent;
    QObject::connect(f, &TcpCamera::captureFinished, f, [=](){
        captureThread.join();
    }, Qt::QueuedConnection);

    provider = new ImageProvider("tcpcamera");

    temperatureData = NULL;
    fps = 0.0;

    thread = new QThread;
    QObject::connect(thread, &QThread::started, [=](){
        exit = false;
        buf.clear();
        socket = new QTcpSocket;
        handshake.disconnect();
        QObject::connect(socket, &QTcpSocket::readyRead, this, &TcpCameraPrivate::onReadyRead, Qt::QueuedConnection);
        QObject::connect(socket, &QTcpSocket::disconnected, [=](){
            handshake.disconnect();
            if( !exit ) {
                qDebug() << "socket disconnect";
                emit f->connectStatusChanged();

                socket->connectToHost(cfg.ip, cfg.port);
                while (!socket->waitForConnected(3000) && !exit) {
//                    emit f->msg(QString("reconnect ip: %1 port: %2").arg(cfg.ip).arg(cfg.port));
                    socket->connectToHost(cfg.ip, cfg.port);
                }
            }
        });
        QObject::connect(this, static_cast<void (TcpCameraPrivate::*)(const QByteArray &)>(&TcpCameraPrivate::write),
                         this, [=](const QByteArray &byte){
            int size = socket->write(byte.data(), byte.size());
            qDebug() << "socket write:" << size;
        }, Qt::QueuedConnection);

        socket->connectToHost(cfg.ip, cfg.port);
        while (!socket->waitForConnected(3000) && !exit) {
//            emit f->msg(QString("reconnect ip: %1 port: %2").arg(cfg.ip).arg(cfg.port));
            socket->connectToHost(cfg.ip, cfg.port);
        }

        if( !exit ) {
            emit f->connectStatusChanged();
        }
    });
    QObject::connect(thread, &QThread::finished, [=](){
        socket->disconnectFromHost();
        socket->deleteLater();
        socket = NULL;
        handshake.disconnect();

        provider->release();
        emit f->videoFrameChanged();
//        emit f->videoFrame(QImage());

        if( temperatureData ) {
            free(temperatureData);
            temperatureData = NULL;
        }

        buf.clear();
        closeUnpack();

        emit f->connectStatusChanged();
    });
    this->moveToThread(thread);

#ifdef Q_OS_ANDROID
    encode = new VideoProcess;
    QObject::connect(encode, &VideoProcess::recordTimeChanged, f, &TcpCamera::recordTimeChanged);
    QObject::connect(encode, &VideoProcess::error, f, [=](){
        encode->closeEncode();
        qDebug() << QString::fromStdString(encode->lastError());
        emit f->msg(tr("Record video fail"));
        emit f->encodingChanged();
    }, Qt::QueuedConnection);
    QObject::connect(encode, &VideoProcess::statusChanged, f, &TcpCamera::encodingChanged);
#endif
}

TcpCameraPrivate::~TcpCameraPrivate()
{
#ifdef Q_OS_ANDROID
    encode->closeEncode();
    encode->deleteLater();
#endif
    saveSetting();

    closeUnpack();
    qDebug() << "tcp release";
}

void TcpCameraPrivate::onReadyRead()
{
    unpackMutex.lock();
    buf.push_back(socket->readAll());
    unpackMutex.unlock();
    if( !handshake.isConnected() ) {
        int res = handshake.recvHandShake(buf.data(), buf.size(), &cfg.cam);
        if( res == (-1) ) {
            buf.remove(0, 1);
        }
        else if( res == 0 ) {
            return;
        }
        else {
            buf.remove(0, res);
//            image = QImage(cfg.cam.w, cfg.cam.h - IMAGE_Y_OFFSET, QImage::Format_RGB888);
            provider->setEmptyRgbImage(cfg.cam.w, cfg.cam.h - IMAGE_Y_OFFSET);

            frameSize = convert().frameSize(cfg.cam.format, cfg.cam.w, cfg.cam.h);
//            emit f->msg(QString("handshake success\n"
//                             "camera width: %1 camera height: %2\n"
//                             "pixel format: %3")
//                     .arg(cfg.cam.w).arg(cfg.cam.h).arg(cfg.cam.format));

            qDebug() << QThread::currentThreadId()
                     << "handshake success"
                     << "camera size:" << cfg.cam.w << "*" << cfg.cam.h
                     << "format:" << cfg.cam.format;

            cfg.set.type = HandShake::__handshake;
            QByteArray byte = handshake.pack(cfg.set);
            int size = socket->write(byte.data(), byte.size());
            qDebug() << "write size:" << size << byte.size();

//            std::string str = handshake.s_pack(cfg.set);
//            int size = socket->write(str.c_str(), str.size());
//            qDebug() << "write size:" << size << str.size();

//            openUnpack();
        }
        return;
    }
    int data_size = frameSize + 4;
    if( buf.size() >= data_size )
    {
        QByteArray tail = buf.mid(data_size - 4, 4);
        if( tail != QByteArray("TcAm") )
        {
            // 包丢失
            // 重新校验包
            qDebug() << "invalid pack" << buf.size() << data_size;
            int i = 0;
            int flag = 0;

            int size = buf.size();
            while (i < size)
            {
                char first = buf.at(i);
                if( first == 'T' ) {
                    tail = buf.mid(i, 4);
                    if( tail == QByteArray("TcAm") )
                    {
                        qDebug() << "remove invalid data, size:" << (i + 1) << data_size;
                        unpackMutex.lock();
                        buf.remove(0, i + 4);
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
                qDebug() << "pack invalid:" << i + 1 << data_size;
                unpackMutex.lock();
                buf.remove(0, i + 1);
                unpackMutex.unlock();
            }
            return;
        }

        frameCount ++;

#ifdef Q_OS_ANDROID
        if( temperatureData == NULL ) {
            temperatureData = (float*)calloc(cfg.cam.w * (cfg.cam.h - IMAGE_Y_OFFSET) + 10, sizeof(float));
        }
        uint16_t *data = reinterpret_cast<uint16_t *>(buf.data());
        uint16_t *temp = data + (cfg.cam.w * (cfg.cam.h - IMAGE_Y_OFFSET));

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
        switch (cfg.cam.w)
        {
        case 384:
            amountPixels=cfg.cam.w*(4-1);
            break;
        case 240:
            amountPixels=cfg.cam.w*(4-3);
            break;
        case 256:
            amountPixels=cfg.cam.w*(4-3);
            break;
        case 640:
            amountPixels=cfg.cam.w*(4-1);
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

        if( (frameCount % 4500) == 25 ) {
            thermometryT4Line(cfg.cam.w,
                              cfg.cam.h,
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

        thermometrySearch(cfg.cam.w,
                          cfg.cam.h,
                          temperatureTable,
                          data,
                          temperatureData,
                          RANGE_MODE,
                          OUTPUT_MODE);

        if( (frameCount % 100) == 0 )
        {
//            qDebug("centerTmp:%.2f, maxTmp:%.2f, minTmp:%.2f, avgTmp:%.2f\n"
//                   "emiss:%.2f, refltmp:%.2f, airtmp:%.2f, humi:%.2f, distance:%d, fix:%.2f\n"
//                   "shufferTemp:%.2f, coreTemp:%.2f",
//                    temperatureData[0],
//                    temperatureData[3],
//                    temperatureData[6],
//                    temperatureData[9],
//                    emiss, Refltmp, Airtmp, humi, distance, correction,
//                    floatShutTemper, floatCoreTemper);
        }

#endif
        timer0 = timer1;
        timer1 = clock();
        uint8_t *bit = provider->data();

        fps = CLOCKS_PER_SEC / (double)(timer1 - timer0);
        if( cfg.cam.format == __yuyv ) {
            convert().yuv422_to_rgb(reinterpret_cast<uint8_t *>(buf.data()), bit,
                                    cfg.cam.w, cfg.cam.h - IMAGE_Y_OFFSET);
        }
        else if( cfg.cam.format == __yuv420 ) {
            convert().yuv420p_to_rgb(reinterpret_cast<uint8_t *>(buf.data()), bit,
                                     cfg.cam.w, cfg.cam.h - IMAGE_Y_OFFSET);
        }

#ifdef Q_OS_ANDROID
        // 画温度信息
        if( showTemp ) {
            QPainter pr(&provider->image());
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
            int cx = cfg.cam.w / 2;
            int cy = cfg.cam.h / 2;
            pr.drawLine(cx - 10, cy, cx + 10, cy);
            pr.drawLine(cx, cy - 10, cx, cy + 10);

            str = QString::number(temperatureData[0], 'f', 1);
            fontW = metrics.horizontalAdvance(str);
            fontH = metrics.height();
            pr.drawText(cx, cy - fontH,
                        fontW, fontH,
                        Qt::AlignCenter,
                        str);
        }

        if( encode->status() ) {
            encode->pushEncode(provider->image());
        }
#endif


        emit f->videoFrameChanged();
//        emit f->videoFrame(provider->image());
        buf.remove(0, data_size);
    }
}

void TcpCameraPrivate::readSetting()
{
#ifdef Q_OS_ANDROID
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QString("/cameraparam.ini");
#else
    QString path = QGuiApplication::applicationDirPath() + QString("/cameraparam.ini");
#endif
    qDebug() << "read setting path:" << path;
    QSettings *s = new QSettings(path, QSettings::IniFormat);

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

    showTemp = s->value("normal/showtemp", false).toBool();
//    emit f->paletteChanged();

    delete s;
    s = NULL;
}

void TcpCameraPrivate::saveSetting()
{
#ifdef Q_OS_ANDROID
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QString("/cameraparam.ini");
#else
    QString path = QGuiApplication::applicationDirPath() + QString("/cameraparam.ini");
#endif
    QSettings *s = new QSettings(path, QSettings::IniFormat);

    s->setValue("Normal/palette", cfg.set.palette);
    s->setValue("Normal/mode", cfg.set.mode);
    s->setValue("Normal/emiss", cfg.set.emiss);
    s->setValue("Normal/reflected", cfg.set.reflected);
    s->setValue("Normal/ambient", cfg.set.ambient);
    s->setValue("Normal/humidness", cfg.set.humidness);
    s->setValue("Normal/correction", cfg.set.correction);
    s->setValue("Normal/distance", cfg.set.distance);

    s->setValue("normal/showtemp", showTemp);

    s->sync();
    delete s;
    s = NULL;

    qDebug() << "save setting path:" << path;
}

void TcpCameraPrivate::capture()
{
    if( !captureThread.joinable() ) {
        captureThread = std::thread([=](){
            QPixmap pix = QPixmap::fromImage(provider->image());
            QString fileName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + QString(".jpg");
#ifdef Q_OS_ANDROID
            QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
#else
            QString path = QGuiApplication::applicationDirPath();
#endif
            if( !path.isEmpty() ) {
                path.append("/Hotimage");
                if( !QFileInfo::exists(path) ) {
                    QDir d;
                    d.mkdir(path);
                }

                path.append("/IMG_" + fileName);
                bool flag = pix.save(path, "JPG");
                if( flag ) {
//                    emit f->msg(QString("capture success %1").arg(path.right(path.length() - path.lastIndexOf('/') - 1)));
                    qDebug() << "capture success:" << path;
                }
                else {
                    emit f->msg(tr("Captrue fail"));
                }
            }
            else {
                qDebug() << "QStandardPaths::writableLocation is empty";
            }
            emit f->captureFinished(path);
        });
    }
}

void TcpCameraPrivate::shutter()
{
    if( handshake.isConnected() ) {
        cfg.set.type = HandShake::__shuffer;
        QByteArray byte = handshake.pack(cfg.set);
        emit write(byte);
    }
}

void TcpCameraPrivate::setPalette(const int &index)
{
    if( handshake.isConnected() ) {
        cfg.set.type = HandShake::__palette;
        cfg.set.palette = index;
        QByteArray byte = handshake.pack(cfg.set);
        emit write(byte);
    }
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
    emit write(byte);
}

void TcpCameraPrivate::openUnpack()
{
    if( unpackThread.joinable() ) {
        return;
    }

    runDecode = 1;
    unpackThread = std::thread([=](){
        int data_size = frameSize + 4;
        while (runDecode)
        {
            while (buf.size() >= data_size) {
                unpack(data_size);
            }
        }
    });
}

void TcpCameraPrivate::unpack(const int &data_size)
{
    QByteArray tail = buf.mid(data_size - 4, 4);
    if( tail != QByteArray("TcAm") )
    {
        // 包丢失
        // 重新校验包
        qDebug() << "invalid pack" << buf.size() << data_size;
        int i = 0;
        int flag = 0;

        while (i < buf.size())
        {
            char first = buf.at(i);
            if( first == 'T' ) {
                tail = buf.mid(i, 4);
                if( tail == QByteArray("TcAm") )
                {
                    qDebug() << "remove invalid data, size:" << (i + 1) << data_size;
                    unpackMutex.lock();
                    buf.remove(0, i + 1);
                    unpackMutex.unlock();
                    flag = 1;
                    break;
                }
            }
            else {
                i ++;
            }
        }

        if( flag == 0 ) {
            // 当前数据无效
            qDebug() << "pack invalid:" << i + 1 << data_size;
            unpackMutex.lock();
            buf.remove(0, i + 1);
            unpackMutex.unlock();
        }

    }

    frameCount ++;

#ifdef Q_OS_ANDROID
    if( temperatureData == NULL ) {
        temperatureData = (float*)calloc(cfg.cam.w * (cfg.cam.h - IMAGE_Y_OFFSET) + 10, sizeof(float));
    }
    uint16_t *data = reinterpret_cast<uint16_t *>(buf.data());
    uint16_t *temp = data + (cfg.cam.w * (cfg.cam.h - IMAGE_Y_OFFSET));

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
    switch (cfg.cam.w)
    {
    case 384:
        amountPixels=cfg.cam.w*(4-1);
        break;
    case 240:
        amountPixels=cfg.cam.w*(4-3);
        break;
    case 256:
        amountPixels=cfg.cam.w*(4-3);
        break;
    case 640:
        amountPixels=cfg.cam.w*(4-1);
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

    if( (frameCount % 4500) == 25 ) {
        thermometryT4Line(cfg.cam.w,
                          cfg.cam.h,
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

    thermometrySearch(cfg.cam.w,
                      cfg.cam.h,
                      temperatureTable,
                      data,
                      temperatureData,
                      RANGE_MODE,
                      OUTPUT_MODE);

    if( (frameCount % 100) == 0 ) {
        qDebug("centerTmp:%.2f, maxTmp:%.2f, minTmp:%.2f, avgTmp:%.2f\n"
               "emiss:%.2f, refltmp:%.2f, airtmp:%.2f, humi:%.2f, distance:%d, fix:%.2f\n"
               "shufferTemp:%.2f, coreTemp:%.2f",
                temperatureData[0],
                temperatureData[3],
                temperatureData[6],
                temperatureData[9],
                emiss, Refltmp, Airtmp, humi, distance, correction,
                floatShutTemper, floatCoreTemper);
    }

#endif
    timer0 = timer1;
    timer1 = clock();
    uint8_t *bit = provider->data();

    fps = CLOCKS_PER_SEC / (double)(timer1 - timer0);
    if( cfg.cam.format == __yuyv ) {
        convert().yuv422_to_rgb(reinterpret_cast<uint8_t *>(buf.data()), bit,
                                cfg.cam.w, cfg.cam.h - IMAGE_Y_OFFSET);
    }
    else if( cfg.cam.format == __yuv420 ) {
        convert().yuv420p_to_rgb(reinterpret_cast<uint8_t *>(buf.data()), bit,
                                 cfg.cam.w, cfg.cam.h - IMAGE_Y_OFFSET);
    }

#ifdef Q_OS_ANDROID
    // 画温度信息
    if( showTemp ) {
        QPainter pr(&provider->image());
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
        int cx = cfg.cam.w / 2;
        int cy = cfg.cam.h / 2;
        pr.drawLine(cx - 10, cy, cx + 10, cy);
        pr.drawLine(cx, cy - 10, cx, cy + 10);

        str = QString::number(temperatureData[0], 'f', 1);
        fontW = metrics.horizontalAdvance(str);
        fontH = metrics.height();
        pr.drawText(cx, cy - fontH,
                    fontW, fontH,
                    Qt::AlignCenter,
                    str);
    }

    if( encode->status() ) {
        encode->pushEncode(provider->image());
    }
#endif


    emit f->videoFrameChanged();
    unpackMutex.lock();
    buf.remove(0, data_size);
    unpackMutex.unlock();
}

void TcpCameraPrivate::closeUnpack()
{
    if( unpackThread.joinable() ) {
        runDecode = 0;
        unpackThread.join();
    }
}

#include "tcpcamera.moc"
