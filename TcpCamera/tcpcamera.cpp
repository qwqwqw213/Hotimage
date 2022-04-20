#include "tcpcamera.h"

#ifdef Q_OS_ANDROID
#include "xtherm/thermometry.h"
#include "UVCamera/uvcamera.h"
#endif

#include "pixeloperations.hpp"
#include "Config/config.h"
#include "VideoProcess/videoprocess.h"
#include "tcpsearcher.hpp"


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

#ifdef Q_OS_ANDROID
    QScopedPointer<UVCamera> uvc;
    static void UvcCallback(void *content, void *data, const int &width, const int &height, const int &bufferLength);
#endif

    VideoProcess *encode;

    tcp_config cfg;
    HandShake handshake;

    QThread *thread;
    QTimer *timer;
    QTcpSocket *socket;
    void socketConnection();
    QByteArray buf;

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

//    QSettings *settings;
    void readSetting();
    void saveSetting();
    void capture();
    void shutter();
    void setPalette(const int &index);

#ifdef Q_OS_ANDROID
    void sendFloatCommand(int position, unsigned char value0, unsigned char value1, unsigned char value2, unsigned char value3);
    void sendUshortCommand(int position, unsigned char value0, unsigned char value1);
    void sendCorrection(float correction);
    void sendReflection(float reflection);
    void sendAmb(float amb);
    void sendHumidity(float humidity);
    void sendEmissivity(float emiss);
    void sendDistance(unsigned short distance);
#endif

    void setCameraParam(const qreal &emiss, const qreal &reflected,
                        const qreal &ambient, const qreal &humidness,
                        const qreal &correction, const int &distance);
    bool isValidIpv4Addres(const QString &ip);

    QMutex unpackMutex;

    QTimer *searchTimer;
    bool manualConnState;
    QVector<TcpSearcher *> searcher;
    void clearSearcher();
    void connectDevice(const QString &dev);
    void searchDevice();

    bool showTemp;

    QString localIP;
    QString deviceIP;

    QString cameraSN;

    int isHotspotMode(const uint8_t &n, const uint8_t &o);

    void decode(uint16_t *data, const int &width, const int &height, const CameraPixelFormat &format, QImage *image);

private:
    TcpCamera *f;

Q_SIGNALS:
    void write(const QByteArray &byte);
    void manualConnect(const QString &devIP);

private Q_SLOTS:
    void onReadyRead();
    void searchOvertime();
    void hostTest();

};

TcpCamera::TcpCamera(QObject *parent)
    : ProviderCamera(parent)
    , p(new TcpCameraPrivate(this))
{
//    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
//    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
//        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
//             qDebug() << localhost << address.toString();
//        }
//    }
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
#ifdef Q_OS_ANDROID
    return p->uvc.isNull() ? p->thread->isRunning() : true;
#else
    return p->thread->isRunning();
#endif
}

int TcpCamera::width()
{
#ifdef Q_OS_ANDROID
    return p->uvc.isNull() ? p->cfg.header.width : p->uvc->width();
#else
    return p->cfg.header.width;
#endif
}

int TcpCamera::height()
{
#ifdef Q_OS_ANDROID
    return p->uvc.isNull() ? p->cfg.header.height - IMAGE_Y_OFFSET : p->uvc->height() - IMAGE_Y_OFFSET;
#else
    return p->cfg.header.height - IMAGE_Y_OFFSET;
#endif
}

int TcpCamera::fps()
{
#ifdef Q_OS_ANDROID
    return p->uvc.isNull() ? p->fps : p->uvc->fps();
#else
    return p->fps;
#endif
}

bool TcpCamera::isConnected()
{
#ifdef Q_OS_ANDROID
    return p->uvc.isNull() ? (p->socket != nullptr) : p->uvc->isOpen();
#else
    return (p->socket != nullptr);
#endif
}

QString TcpCamera::localIp()
{
    return p->localIP;
}

QString TcpCamera::deviceIp()
{
    return p->deviceIP;
}

bool TcpCamera::manaulConnectState()
{
    return p->manualConnState;
}

void TcpCamera::manualConnect(const QString &devIp)
{
    if( isConnected() ) {
        emit msg(tr("Is connected state"));
        return;
    }
    if( !p->isValidIpv4Addres(devIp) ) {
        emit msg(tr("Invalid IP"));
        return;
    }
    emit p->manualConnect(devIp);
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

void TcpCamera::capture()
{
    if( !isConnected() ) {
        return;
    }
    p->capture();
}

void TcpCamera::shutter()
{
    if( !isConnected() ) {
        return;
    }
    p->shutter();
}

void TcpCamera::setPalette(const int &index)
{
    if( !isConnected() ) {
        return;
    }
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
        cfg.width = width();
        cfg.height = height();
        cfg.fps = fps();
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

bool TcpCamera::hotspotMode()
{
    return p->cfg.set.hotspotMode;
}

QString TcpCamera::hotspotSSID()
{
    return QString::fromStdString(p->cfg.set.hotspot_ssid);
}

QString TcpCamera::hotspotPassword()
{
    return QString::fromStdString(p->cfg.set.hotspot_password);
}

/*
 *  热点模式流程
 *  手机设置热点ssid password后
 *  打开热点模式
 *  发送热点信息到路由器
 *  路由器接收后，更新热点模式(hotspot_mode 标志位)
 *  在通过画面帧获取到新的hotspot_mode 标志位
 *  确认路由器是否打开热点模式
 *  打开热点模式后, 则开始搜索ip
 */
bool TcpCamera::setHotspotParam(const QString &ssid, const QString &password)
{
    if( !ssid.isEmpty() && !password.isEmpty() ) {
        p->cfg.set.hotspot_ssid = ssid.toStdString();
        p->cfg.set.hotspot_password = password.toStdString();
        p->cfg.set.type = HandShake::__hotspot_info;
        QByteArray byte = p->handshake.pack(p->cfg.set);
        emit p->write(byte);
    }
    return true;
}

void TcpCamera::saveSetting()
{
    p->saveSetting();
}

void TcpCamera::openUsbCamera(const int &fd)
{
#ifdef Q_OS_ANDROID
    closeUsbCamera();
    p->uvc.reset(new UVCamera);
    QObject::connect(p->uvc.data(), &UVCamera::updateCameraState, [=](){
        qDebug() << "camera state update:" << p->uvc->isOpen();
        if( p->uvc->isOpen() ) {
            p->uvc->zoomAbsolute(0x8005);
            p->setPalette(p->cfg.set.palette);
            close();
        }
    });
    p->uvc->open(fd, __pix_yuyv,
                 TcpCameraPrivate::UvcCallback, p.data());
#endif
}

void TcpCamera::closeUsbCamera()
{
#ifdef Q_OS_ANDROID
    if( !p->uvc.isNull() ) {
        p->uvc->close();
        open();
    }
#endif
}

TcpCameraPrivate::TcpCameraPrivate(TcpCamera *parent)
    : QObject(nullptr)
{
    readSetting();

    exit = 1;
    f = parent;
    QObject::connect(f, &TcpCamera::captureFinished, f, [=](){
        if( captureThread.joinable() ) {
            captureThread.join();
        }
    }, Qt::QueuedConnection);

#ifdef TEMPERATURE_SDK
    temperatureData = NULL;
#endif
    fps = 0.0;
    manualConnState = false;

    thread = new QThread;
    QObject::connect(thread, &QThread::started, [=](){
        exit = 0;
        buf.clear();
        timer = new QTimer;
        timer->setInterval(2000);
        timer->setSingleShot(true);

        cameraSN = QString("");
        socket = nullptr;

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

        QObject::connect(this, static_cast<void (TcpCameraPrivate::*)(const QString &)>(&TcpCameraPrivate::manualConnect),
                         this, [=](const QString &devIp){
            connectDevice(devIp);
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

        qDebug() << QThread::currentThreadId() << "start search socket";
        searchTimer = new QTimer;
        QObject::connect(searchTimer, &QTimer::timeout, this, &TcpCameraPrivate::searchOvertime);
        searchDevice();
    });

    QObject::connect(thread, &QThread::finished, [=](){
        clearSearcher();

        timer->stop();
        timer->deleteLater();
        timer = nullptr;

        if( socket ) {
            cfg.set.type = HandShake::__disconnect;
            QByteArray byte = handshake.pack(cfg.set);
            socket->write(byte);
            bool flag = socket->waitForBytesWritten(10 * 1000);
            qDebug() << "send disconnect:" << flag;

            socket->deleteLater();
            socket = nullptr;
        }

        if( f->encoding() ) {
            f->closeRecord();
        }

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
    clearSearcher();

//#ifdef Q_OS_ANDROID
    encode->closeEncode();
    encode->deleteLater();
//#endif
}

void TcpCameraPrivate::socketConnection()
{
    if( socket == nullptr ) {
        return;
    }

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
        if( socket != nullptr ) {
            qDebug() << "socket disconnect:" << socket->state();
            emit f->connectStatusChanged();
            unpackMutex.lock();
            buf.clear();
            unpackMutex.unlock();

            // 此信号有可能为上次接收, 此时socket已重新连接
            if( socket->state() != QTcpSocket::ConnectedState ) {
                cameraSN = QString("");
                socket->deleteLater();
                socket = nullptr;
                f->clearImage();
                searchDevice();
            }
        }
    }, Qt::QueuedConnection);

    qDebug() << "first time request frame";
    timer->start();
    cfg.set.type = HandShake::__handshake;
    QByteArray byte = handshake.pack(cfg.set);
//    socket->write(byte);
    emit write(byte);
}

void TcpCameraPrivate::clearSearcher()
{
    for(const auto &s : searcher) {
        s->release();
        s->deleteLater();
    }
    searcher.clear();
}

void TcpCameraPrivate::connectDevice(const QString &dev)
{
    if( dev.isEmpty() ) {
        qDebug() << "ERROR: connect device ip is empty";
        return;
    }
    if( socket != nullptr ) {
        qDebug() << "ERROR: socket exist";
        return;
    }

    searchTimer->stop();
    clearSearcher();

    manualConnState = true;
    emit f->manualConnectStateChanged();

    socket = new QTcpSocket;

    int time = 0;
    while ((exit != 1) && (socket != nullptr) && (time < 30)) {
        socket->connectToHost(dev, cfg.port);
        socket->waitForConnected(3000);
        if( socket->state() == QTcpSocket::ConnectedState ) {
            deviceIP = dev;
            emit f->msg("Manual connect success");
            break;
        }
        time ++;
    }

    manualConnState = false;
    emit f->manualConnectStateChanged();

    if( socket->state() != QTcpSocket::ConnectedState ) {
        emit f->msg("Manual connect fail");
        socket->deleteLater();
        socket = nullptr;
        searchDevice();
    }
    else {
        emit f->msg("Connect success");
    }
}

void TcpCameraPrivate::hostTest()
{

}

void TcpCameraPrivate::searchDevice()
{
    if( searcher.size() > 0 ) {
        return;
    }
    /*
     *  在ios里需要获取本地网络使用权限
     *  否则socket无法连接到板子
     */
#ifdef Q_OS_IOS
    QHostInfo::lookupHost("test.com", this, &TcpCameraPrivate::hostTest);
#endif

    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            TcpSearcher *s = new TcpSearcher(address.toString(), cfg.port);
            QObject::connect(s, &TcpSearcher::finished,
                             this, [=](const QString &devIp, const QString &locIp){
                qDebug() << "found device ip:" << devIp << thread->currentThreadId() << thread;

                deviceIP = devIp;
                localIP = locIp;
                socket = static_cast<TcpSearcher *>(sender())->socket();
                socketConnection();

                clearSearcher();
            }, Qt::QueuedConnection);

            searcher.append(s);
        }
    }

//    QTimer::singleShot(20 * 1000, this, &TcpCameraPrivate::searchOvertime);
    searchTimer->start(10 * 1000);
}

void TcpCameraPrivate::searchOvertime()
{
    if( manualConnState ) {
        return;
    }
    clearSearcher();

    if( socket == nullptr ) {
        qDebug() << "search socket overtime";
        searchDevice();
    }
}

// hotspot mode
// 状态改变返回 0 or 1
// 0: off, 1: on
// 无状态改变返回 -1
int TcpCameraPrivate::isHotspotMode(const uint8_t &n, const uint8_t &o)
{
    int value = (n >> 1) & 0x01;
    if( ((o >> 1) & 0x01) != value ) {
        if( value == 1 ) {
            return 1;
        }
        return 0;
    }
    return -1;
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

    t_header header;
    memcpy(&header, buf.data(), headerSize);
    if( !header.isValidHeader() ) {
        // 包丢失
        // 重新校验包
        qDebug() << "invalid pack";

        int i = 0;
        int flag = 0;

        int size = buf.size();
        while (i < size)
        {
            char first = buf.at(i);
            if( first == 'T' )
            {
                QByteArray head = buf.mid(i, headerSize);
                if( header.isValidHeader(head.data()) )
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
        return;
    }

    int totalSize = header.bufferLength + headerSize;
    if( buf.size() >= totalSize )
    {
        int state = isHotspotMode(header.value, cfg.header.value);
        memcpy(&cfg.header, &header, sizeof (t_header));

        QImage *image = f->rgb();
        CameraPixelFormat format = cfg.header.pixelFormat == __yuyv ? __pix_yuyv : __pix_yuv420p;
        decode(reinterpret_cast<uint16_t *>(buf.data() + headerSize), cfg.header.width, cfg.header.height, format, image);

        /*
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
            emit f->paletteChanged();
            emit f->connectStatusChanged();
            emit f->cameraParamChanged();
        }

        QImage *image = f->rgb();
        uint8_t *bit = image->bits();


        if( cfg.header.pixelFormat == __yuyv ) {
            PixelOperations::yuv422_to_rgb(reinterpret_cast<uint8_t *>(data), bit,
                                           cfg.header.width, cfg.header.height - IMAGE_Y_OFFSET);
        }
        else if( cfg.header.pixelFormat == __yuv420 ) {
            PixelOperations::yuv420p_to_rgb(reinterpret_cast<uint8_t *>(data), bit,
                                            cfg.header.width, cfg.header.height - IMAGE_Y_OFFSET);
        }

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
        if( encode->status() ) {
            encode->pushEncode(image->copy());
        }
        */

        unpackMutex.lock();
        buf.remove(0, totalSize);
        unpackMutex.unlock();

        if( exit == 0 ) {
            timer->start();
            cfg.set.type = HandShake::__handshake;
            QByteArray byte = handshake.pack(cfg.set);
            socket->write(byte);
        }

//        QImage img = image->copy();
//        f->setUrlImage(img);

        // 热点模式被打开
        if( (state >= 0) && (cfg.set.hotspotMode != state) ) {
            cfg.set.hotspotMode = state;
            emit f->hotspotParamChanged();
        }
    }
}

void TcpCameraPrivate::decode(uint16_t *data, const int &width, const int &height, const CameraPixelFormat &format, QImage *image)
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

    uint16_t *temp = data + (width * (height - IMAGE_Y_OFFSET));
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
    switch (width)
    {
    case 384:
        amountPixels=width*(4-1);
        break;
    case 240:
        amountPixels=width*(4-3);
        break;
    case 256:
        amountPixels=width*(4-3);
        break;
    case 640:
        amountPixels=width*(4-1);
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
        thermometryT4Line(width,
                          height,
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

    thermometrySearch(width,
                      height,
                      temperatureTable,
                      data,
                      temperatureData,
                      RANGE_MODE,
                      OUTPUT_MODE);
#endif
    if( cameraSN.isEmpty() ) {
        cameraSN = QString(sn);
        emit f->cameraSNChanged();
        emit f->paletteChanged();
        emit f->connectStatusChanged();
        emit f->cameraParamChanged();
    }
    else {
        cameraSN = QString(sn);
        emit f->cameraSNChanged();
    }

    uint8_t *bit = image->bits();
    if( format == __pix_yuyv ) {
        PixelOperations::yuv422_to_rgb(reinterpret_cast<uint8_t *>(data), bit,
                                       width, height - IMAGE_Y_OFFSET);
    }
    else if( format == __pix_yuv420p ) {
        PixelOperations::yuv420p_to_rgb(reinterpret_cast<uint8_t *>(data), bit,
                                        width, height - IMAGE_Y_OFFSET);
    }

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
        int cx = width / 2;
        int cy = height / 2;
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
    if( encode->status() ) {
        encode->pushEncode(image->copy());
    }

    QImage i = image->copy();
    f->setUrlImage(i);
}

void TcpCameraPrivate::readSetting()
{
    deviceIP = g_Config->readSetting("Tcp/DevceIP", "192.168.1.1").toString();
    cfg.ip = SERVER_IP;
    cfg.port = SERVER_PORT;
    cfg.set.palette = g_Config->readSetting("Tcp/palette", 0).toUInt();
    cfg.set.mode = g_Config->readSetting("Tcp/mode", CAM_OUTPUT_YUYV_MODE).toUInt();
    cfg.set.emiss = g_Config->readSetting("Tcp/emiss", 1.0).toDouble();
    cfg.set.reflected = g_Config->readSetting("Tcp/reflected", 1.0).toDouble();
    cfg.set.ambient = g_Config->readSetting("Tcp/ambient", 1.0).toDouble();
    cfg.set.humidness = g_Config->readSetting("Tcp/humidness", 1.0).toDouble();
    cfg.set.correction = g_Config->readSetting("Tcp/correction", 1.0).toDouble();
    cfg.set.distance = g_Config->readSetting("Tcp/distance", 0).toUInt();
    showTemp = g_Config->readSetting("Tcp/showtemp", false).toBool();
    cfg.set.hotspotMode = g_Config->readSetting("Tcp/hotspotMode", false).toBool();
    cfg.set.hotspot_ssid = g_Config->readSetting("Tcp/hotspotSsid", "").toString().toStdString();
    cfg.set.hotspot_password = g_Config->readSetting("Tcp/hotspotPassword", "").toString().toStdString();
}

void TcpCameraPrivate::saveSetting()
{
    QVector<SettingParam> param;
    param.append({"Tcp/DeviceIP", deviceIP});
    param.append({"Tcp/palette", cfg.set.palette});
    param.append({"Tcp/mode", cfg.set.mode});
    param.append({"Tcp/emiss", cfg.set.emiss});
    param.append({"Tcp/reflected", cfg.set.reflected});
    param.append({"Tcp/ambient", cfg.set.ambient});
    param.append({"Tcp/humidness", cfg.set.humidness});
    param.append({"Tcp/correction", cfg.set.correction});
    param.append({"Tcp/distance", cfg.set.distance});
    param.append({"Tcp/showtemp", showTemp});
    param.append({"Tcp/hotspotMode", cfg.set.hotspotMode});
    param.append({"Tcp/hotspotSsid", QString::fromStdString(cfg.set.hotspot_ssid)});
    param.append({"Tcp/hotspotPassword", QString::fromStdString(cfg.set.hotspot_password)});
    g_Config->insertSetting(param);
}

void TcpCameraPrivate::capture()
{
    if( !captureThread.joinable() ) {
        captureThread = std::thread([=](){
            QImage *rgb = f->rgb();
            QPixmap pix = QPixmap::fromImage(rgb->copy());
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
#ifdef Q_OS_ANDROID
    if( !uvc.isNull() ) {
        uvc->zoomAbsolute(0x8000);
        return;
    }
#endif
    cfg.set.type = HandShake::__shuffer;
    QByteArray byte = handshake.pack(cfg.set);
    emit write(byte);
}

void TcpCameraPrivate::setPalette(const int &index)
{
    cfg.set.palette = index;
#ifdef Q_OS_ANDROID
    if( !uvc.isNull() ) {
        uvc->zoomAbsolute(0x8800 | (index & 0xfff));
        return;
    }
#endif
    cfg.set.type = HandShake::__palette;
    QByteArray byte = handshake.pack(cfg.set);
    emit write(byte);
}

#ifdef Q_OS_ANDROID
void TcpCameraPrivate::sendFloatCommand(int position, unsigned char value0, unsigned char value1, unsigned char value2, unsigned char value3)
{
    int psitionAndValue0 = (position << 8) | (0x000000ff & value0);
    if( uvc->zoomAbsolute(psitionAndValue0) < 0 ) {
        return;
    }
    int psitionAndValue1 = ((position + 1) << 8) | (0x000000ff & value1);
    if( uvc->zoomAbsolute(psitionAndValue1) < 0 ) {
        return;
    }
    int psitionAndValue2 = ((position + 2) << 8) | (0x000000ff & value2);
    if( uvc->zoomAbsolute(psitionAndValue2) < 0 ) {
        return;
    }
    int psitionAndValue3 = ((position + 3) << 8) | (0x000000ff & value3);
    if( uvc->zoomAbsolute(psitionAndValue3) < 0 ) {
        return;
    }
}

void TcpCameraPrivate::sendUshortCommand(int position, unsigned char value0, unsigned char value1)
{
    int psitionAndValue0 = (position << 8) | (0x000000ff & value0);
    if( uvc->zoomAbsolute(psitionAndValue0) < 0 ) {
        return;
    }
    int psitionAndValue1 = ((position + 1) << 8) | (0x000000ff & value1);
    if( uvc->zoomAbsolute(psitionAndValue1) < 0 ) {
        return;
    }
}

void TcpCameraPrivate::sendCorrection(float correction)
{
    unsigned char iputCo[4];
    memcpy(iputCo,&correction,sizeof(float));
    sendFloatCommand(0 * 4, iputCo[0], iputCo[1], iputCo[2], iputCo[3]);
}

void TcpCameraPrivate::TcpCameraPrivate::sendReflection(float reflection)
{
    unsigned char iputRe[4];
    memcpy(iputRe,&reflection,sizeof(float));
    sendFloatCommand(1 * 4, iputRe[0], iputRe[1], iputRe[2], iputRe[3]);
}

void TcpCameraPrivate::sendAmb(float amb)
{
    unsigned char iputAm[4];
    memcpy(iputAm,&amb,sizeof(float));
    sendFloatCommand(2 * 4, iputAm[0], iputAm[1], iputAm[2], iputAm[3]);

}

void TcpCameraPrivate::sendHumidity(float humidity)
{
    unsigned char iputHu[4];
    memcpy(iputHu,&humidity,sizeof(float));
    sendFloatCommand(3 * 4, iputHu[0], iputHu[1], iputHu[2], iputHu[3]);

}

void TcpCameraPrivate::sendEmissivity(float emiss)
{
    unsigned char iputEm[4];
    memcpy(iputEm,&emiss,sizeof(float));
    sendFloatCommand(4 * 4, iputEm[0], iputEm[1], iputEm[2], iputEm[3]);
}

void TcpCameraPrivate::sendDistance(unsigned short distance)
{
    unsigned char iputDi[2];
    memcpy(iputDi,&distance,sizeof(unsigned short));
    sendUshortCommand(5 * 4, iputDi[0], iputDi[1]);
}
#endif

void TcpCameraPrivate::setCameraParam(const qreal &emiss, const qreal &reflected,
                                      const qreal &ambient, const qreal &humidness,
                                      const qreal &correction, const int &distance)
{
    cfg.set.emiss = emiss;
    cfg.set.reflected = reflected;
    cfg.set.ambient = ambient;
    cfg.set.humidness = humidness;
    cfg.set.correction = correction;
    cfg.set.distance = distance;
    emit f->cameraParamChanged();
#ifdef Q_OS_ANDROID
    if( !uvc.isNull() ) {
        sendEmissivity(emiss);
        QThread::msleep(15);
        sendReflection(reflected);
        QThread::msleep(15);
        sendAmb(ambient);
        QThread::msleep(15);
        sendHumidity(humidness);
        QThread::msleep(15);
        sendCorrection(correction);
        QThread::msleep(15);
        sendDistance(distance);
        return;
    }
#endif
    cfg.set.type = HandShake::__config;
    QByteArray byte = handshake.pack(cfg.set);
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

#ifdef Q_OS_ANDROID
void TcpCameraPrivate::UvcCallback(void *content, void *data, const int &width, const int &height, const int &bufferLength)
{

    TcpCameraPrivate *cam = static_cast<TcpCameraPrivate *>(content);

    CameraPixelFormat format = cam->uvc->pixelFormat();
    QImage *rgb = cam->f->rgb();
    cam->decode(reinterpret_cast<uint16_t *>(data), width, height, format, rgb);

//    switch (format) {
//    case __pix_yuyv: {
//        QImage *rgb = cam->f->rgb();
//        PixelOperations::yuv422_to_rgb(reinterpret_cast<uint8_t *>(data), rgb->bits(), width, height - IMAGE_Y_OFFSET);
//        QImage img = rgb->copy();
//        cam->f->setUrlImage(img);
//    }
//        break;
//    case __pix_mjpeg: {
//        QImage jpg = QImage::fromData(reinterpret_cast<uint8_t *>(data), bufferLength);
//        cam->f->setUrlImage(jpg);
//    }
//        break;
//    default: break;
//    }


}
#endif

#include "tcpcamera.moc"
