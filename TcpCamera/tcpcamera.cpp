#include "tcpcamera.h"

#ifdef Q_OS_ANDROID
#include "xtherm/thermometry.h"
#include "UVCamera/uvcamera.h"
#endif

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
#include "functional"

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

    QThread *thread;

    int sendOvertimeCount;
    QTimer *sendTimer;
    hs::t_packet packet(const hs::RequestType &req);
    QByteArray requestByte(hs::t_packet *p);
    void requestFrame(const hs::RequestType &type = hs::__req_frame);

    QScopedPointer<QTcpSocket> socket;
    void socketConnection();
    QMutex buffetMutex;
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
    void setFrameMode(const int &mode);
    void setCameraParam(const qreal &emiss, const qreal &reflected,
                        const qreal &ambient, const qreal &humidness,
                        const qreal &correction, const int &distance);
    bool setHotspotParam(const QString &ssid, const QString &password);

    bool isValidIpv4Addres(const QString &ip);

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

    QMutex frameBufferMutex;
    QScopedPointer<uint8_t> frameBuffer;
    CameraPixelFormat toCameraPixel(const hs::PixelFormat &format);
    void decode(uint8_t *buf,
                const int &width, const int &height,
                const CameraPixelFormat &format,
                QImage *image,
                const hs::FrameFormat &frameFormat);

    void decode_tiny(uint8_t *buf,
                     const int &width, const int &height,
                     QImage *image,
                     const hs::FrameFormat &frameFormat);

    void y2_to_rgb24(const uint8_t *in, uint8_t *out, const int &width, const int &height);
    void nuc16_to_rgb24(const uint16_t *nuc, uint8_t *rgb, const int &width, const int &height);

    QScopedPointer<QTcpServer> server;
    void openServer();
    void closeServer();

private:
    TcpCamera *f;

    void onUnpacket();

    class Unpacket {
    public:
        Unpacket(const std::function<void()> &_f) {
            f = _f;
            t = std::thread([=](){
                exit = false;
                while (!exit) {
                    std::unique_lock<std::mutex> l(this->m);
                    n = false;
                    while (!n) {
                        c.wait(l);
                    }
                    l.unlock();

                    f();
                }
            });
        }
        ~Unpacket() { }
        void notify() { n = true; c.notify_all(); }
        void close() { exit = true; notify(); t.join(); }

    private:
        std::function<void()> f;
        std::thread t;
        std::mutex m;
        std::condition_variable c;
        bool n;
        bool exit;
    };
    QScopedPointer<Unpacket> unpacket;

Q_SIGNALS:
    void restartSendTimer();
    void write(const QByteArray &byte);
    void manualConnect(const QString &devIP);

private Q_SLOTS:
    void onReadyRead();
    void searchOvertime();

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

    pixelOperations()->initializerPseudo((PseudoColorTable)p->cfg.palette);
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
    return p->uvc.isNull() ? p->cfg.width : p->uvc->width();
#else
    return p->cfg.width;
#endif
}

int TcpCamera::height()
{
#ifdef Q_OS_ANDROID
//    return p->uvc.isNull() ? p->cfg.height - IMAGE_Y_OFFSET : p->uvc->height() - IMAGE_Y_OFFSET;
    if( p->uvc.isNull() ) {
        switch (p->cfg.devType) {
        case hs::__t2l_a4l: { return (p->cfg.height - IMAGE_Y_OFFSET); }
        case hs::__tiny_1b: { return (p->cfg.height / 2); }
        }
        return 0;
    }
    else {
        return p->uvc->height() - IMAGE_Y_OFFSET;
    }

#else
//    return p->cfg.height - IMAGE_Y_OFFSET;
    switch (p->cfg.devType) {
    case hs::__t2l_a4l: { return (p->cfg.height - IMAGE_Y_OFFSET); }
    case hs::__tiny_1b: { return (p->cfg.height / 2); }
    }
    return 0;
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
    return p->uvc.isNull() ? (isOpen() ? (!p->socket.isNull()) : false) : p->uvc->isOpen();
#else
    return (!p->socket.isNull());
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
    if( p->manualConnState ) {
        emit msg(tr("Searching"));
        return;
    }

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
    if( isOpen() ) {
        qDebug() << "tcp start close";
        if( encoding() ) {
            closeRecord();
        }
        p->exit = 1;
        p->thread->quit();
        p->thread->wait();
        emit connectStatusChanged();
        g_Config->appendLog(QString("tcp thread close"));
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
}

void TcpCamera::setCameraParam(const qreal &emiss, const qreal &reflected,
                               const qreal &ambient, const qreal &humidness,
                               const qreal &correction, const int &distance)
{
    p->setCameraParam(emiss, reflected, ambient, humidness, correction, distance);
}

int TcpCamera::palette()
{
    return p->cfg.palette;
}

qreal TcpCamera::emiss()
{
    return p->cfg.emiss;
}

qreal TcpCamera::reflected()
{
    return p->cfg.reflected;
}

qreal TcpCamera::ambient()
{
    return p->cfg.ambient;
}

qreal TcpCamera::humidness()
{
    return p->cfg.humidness;
}

qreal TcpCamera::correction()
{
    return p->cfg.correction;
}

uint16_t TcpCamera::distance()
{
    return p->cfg.distance;
}

int TcpCamera::frameMode()
{
    return p->cfg.frameMode;
}

void TcpCamera::setFrameMode(const int &mode)
{
    p->setFrameMode(mode);
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

QString TcpCamera::version()
{
    return p->socket.isNull() ? "" : p->cfg.version;
}

bool TcpCamera::hotspotEnable()
{
    return p->cfg.hotspotEnable;
}

QString TcpCamera::hotspotSSID()
{
    return p->cfg.hotspotSsid;
}

QString TcpCamera::hotspotPassword()
{
    return p->cfg.hotspotPassword;
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
    return p->setHotspotParam(ssid, password);
}

int TcpCamera::infraredState()
{
    return p->cfg.infraredState;
}

void TcpCamera::setInfraredState(const int &state)
{
    hs::t_packet packet = p->packet(hs::__req_infrared);
    packet.infraredState = state;
    emit p->write(p->requestByte(&packet));
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
        g_Config->appendLog(QString("camera state update: %1").arg(p->uvc->isOpen()));
        if( p->uvc->isOpen() ) {
            p->uvc->zoomAbsolute(XTHERM_N16_MODE);
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

    deviceIP = QString("");

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

        cameraSN = QString("");
//        socket = nullptr;

        QObject::connect(this, static_cast<void (TcpCameraPrivate::*)(const QByteArray &)>(&TcpCameraPrivate::write),
                         this, [=](const QByteArray &byte){
            if( socket.isNull() ) {
                hs::t_packet packet;
                memcpy(&packet, byte.data(), sizeof(hs::t_packet));
                qDebug() << "socket is null, request:" << packet.request;
                return ;
            }

            if( socket->state() == QTcpSocket::ConnectedState ) {
                int size = socket->write(byte.data(), byte.size());
//                g_Config->appendLog(QString("socket write size: %1").arg(size));
            }
        }, Qt::QueuedConnection);

        QObject::connect(this, static_cast<void (TcpCameraPrivate::*)(const QString &)>(&TcpCameraPrivate::manualConnect),
                         this, [=](const QString &devIp){
            connectDevice(devIp);
        }, Qt::QueuedConnection);

        sendTimer = new QTimer;
        sendTimer->setTimerType(Qt::PreciseTimer);
        sendTimer->setInterval(2000);
        sendTimer->setSingleShot(true);
        QObject::connect(sendTimer, &QTimer::timeout, [=](){
            qDebug() << "request frame time out:" << socket.data();
            if( socket.isNull() ) {
                return ;
            }

            sendOvertimeCount ++;
            if( sendOvertimeCount > 25 ) {
                sendOvertimeCount = 0;
                emit f->msg(tr("Abnormal communication with the camera"));
            }

            // 超时重新请求
            requestFrame();
        });
        QObject::connect(this, &TcpCameraPrivate::restartSendTimer, this, [=](){
            sendTimer->start();
        }, Qt::QueuedConnection);

        g_Config->appendLog(QString("start search socket"));
        searchTimer = new QTimer;
        searchTimer->setTimerType(Qt::PreciseTimer);
        QObject::connect(searchTimer, &QTimer::timeout, this, &TcpCameraPrivate::searchOvertime);

        unpacket.reset(new Unpacket(std::bind(&TcpCameraPrivate::onUnpacket, this)));

        if( !deviceIP.isEmpty() ) {
            qDebug() << "try prev connected device ip:" << deviceIP;
            socket.reset(new QTcpSocket);
            socket->connectToHost(deviceIP, cfg.port);
            socket->waitForConnected(3000);
            if( socket->state() == QTcpSocket::ConnectedState ) {
                qDebug() << "reconnect success";
                emit f->connectStatusChanged();
                socketConnection();
            }
            else {
                socket.reset();
                searchDevice();
            }
//            connectDevice(deviceIP);

        }
        else {
            qDebug() << "first search device";
            searchDevice();
        }

        openServer();
    });

    QObject::connect(thread, &QThread::finished, [=](){
        closeServer();

        clearSearcher();

        unpacket->close();
        qDebug() << "close unpacket";
        unpacket.reset();

        sendTimer->stop();
        sendTimer->deleteLater();
        sendTimer = nullptr;

        if( !socket.isNull() ) {
            hs::t_packet p = packet(hs::__req_disconnect);
            socket->write(requestByte(&p));
            bool flag = socket->waitForBytesWritten(5 * 1000);
            g_Config->appendLog(QString("send disconnect: %1").arg(flag));
            socket->disconnect();
            socket.reset();
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

        if( !frameBuffer.isNull() ) {
            frameBuffer.reset();
        }

        g_Config->appendLog(QString("socket release"));
    });

    this->moveToThread(thread);

    encode = new VideoProcess;
    QObject::connect(encode, &VideoProcess::recordTimeChanged, f, &TcpCamera::recordTimeChanged);
    QObject::connect(encode, &VideoProcess::error, f, [=](){
        encode->closeEncode();
        g_Config->appendLog(QString::fromStdString(encode->lastError()));
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

hs::t_packet TcpCameraPrivate::packet(const hs::RequestType &req)
{
    hs::t_packet p;
    p.marker[0] = 'R';
    p.marker[1] = 'e';
    p.marker[2] = 'C';
    p.marker[3] = 'v';
    p.request = req;
    p.timestamp = QDateTime::currentMSecsSinceEpoch();
    return p;
}

QByteArray TcpCameraPrivate::requestByte(hs::t_packet *p)
{
    QByteArray byte;
    byte.resize(HANDSHAKE_PACK_SIZE);
    memcpy(byte.data(), p, sizeof (hs::t_packet));
    return byte;
}

void TcpCameraPrivate::requestFrame(const hs::RequestType &type)
{
    if( exit == 1 ) {
        return;
    }

    hs::t_packet p = packet(type);
    if( type == hs::__req_init ) {
        // frame mode为hs::__image_first 时, 使用0x8004模式
        // hs::__fps_first 时, 使用0x8005模式
        p.frameMode = cfg.frameMode;

        // 摄像头参数
        p.emiss = cfg.emiss;
        p.reflected = cfg.reflected;
        p.ambient = cfg.ambient;
        p.humidness = cfg.humidness;
        p.correction = cfg.correction;
        p.distance = cfg.distance;

        QByteArray ssid = cfg.hotspotSsid.toUtf8();
        QByteArray password = cfg.hotspotPassword.toUtf8();
        memcpy(p.hotspotSSID, ssid.data(), ssid.size());
        p.hotspotSSID[ssid.size()] = '\0';
        memcpy(p.hotspotPassword, password.data(), password.size());
        p.hotspotPassword[password.size()] = '\0';

        p.infraredState = cfg.infraredState;
    }
    cfg.frameFormat = (cfg.frameMode == hs::__image_first ? hs::__full_frame : !cfg.frameFormat);
    p.frameFormat = cfg.frameFormat;
    emit restartSendTimer();
    emit write(requestByte(&p));
}

void TcpCameraPrivate::socketConnection()
{
    if( socket.isNull() ) {
        return;
    }

    QObject::connect(socket.data(), &QTcpSocket::readyRead, this, &TcpCameraPrivate::onReadyRead, Qt::QueuedConnection);
    QObject::connect(socket.data(), &QTcpSocket::stateChanged,
                     this, [=](QAbstractSocket::SocketState state){
        if( socket.isNull() ) {
            return ;
        }

        qDebug() << "socket state changed:" << state;
        if( state == QAbstractSocket::ConnectedState ) {
            // 异常断开, 重新请求
            requestFrame(hs::__req_init);
        }
    }, Qt::QueuedConnection);

    QObject::connect(socket.data(), &QTcpSocket::disconnected, this, [=](){
        if( !socket.isNull() ) {
            qDebug() << "socket disconnect:" << socket->state();
            emit f->connectStatusChanged();
            buffetMutex.lock();
            buf.clear();
            buffetMutex.unlock();

            // 此信号有可能为上次接收, 此时socket已重新连接
            if( socket->state() != QTcpSocket::ConnectedState ) {
                cameraSN = QString("");
                socket.reset();
                f->clearImage();
                searchDevice();
            }
        }
    }, Qt::QueuedConnection);

    qDebug() << "first time request frame";
    requestFrame(hs::__req_init);

    emit f->connectStatusChanged();
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
    if( !socket.isNull() ) {
        qDebug() << "ERROR: socket exist";
        return;
    }

    searchTimer->stop();
    clearSearcher();

    manualConnState = true;
    emit f->manualConnectStateChanged();

    socket.reset(new QTcpSocket);

    qDebug() << "manual connect ip:" << dev;

    int time = 0;
    while ((exit != 1) && !socket.isNull() && (time < 5)) {
        socket->connectToHost(dev, cfg.port);
        socket->waitForConnected(1000);
        if( socket->state() == QTcpSocket::ConnectedState ) {
            deviceIP = dev;
            emit f->msg(tr("Manual connect success"));
            break;
        }
        time ++;
    }

    manualConnState = false;
    emit f->manualConnectStateChanged();

    qDebug() << "manual:" << socket->state();
    if( socket->state() != QTcpSocket::ConnectedState ) {
        emit f->msg("Manual connect fail");
        socket.reset();
        searchDevice();
    }
    else {
        qDebug() << "manual conenct success";

        socketConnection();
    }
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
//                socket = static_cast<TcpSearcher *>(sender())->socket();
                socket.reset(static_cast<TcpSearcher *>(sender())->socket());
                socketConnection();
                emit f->connectStatusChanged();
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
    if( manualConnState || !socket.isNull() ) {
        return;
    }
    clearSearcher();

    if( socket.isNull() ) {
        qDebug() << "search socket overtime";
        searchDevice();
    }
}

void TcpCameraPrivate::onReadyRead()
{
    if( sendTimer->isActive() ) {
        sendTimer->stop();
    }

    buffetMutex.lock();
    buf.append(socket->readAll());
    buffetMutex.unlock();

//    unpacket->notify();
    onUnpacket();
}

void TcpCameraPrivate::onUnpacket()
{
    int headerSize = sizeof (hs::t_header);
    if( buf.size() < headerSize ) {
        return;
    }

    hs::t_header header;
    memcpy(&header, buf.data(), headerSize);
    if( !header.isValidHeader() ) {
        // 包丢失
        // 重新校验包
//        qDebug() << "invalid pack";

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
                    qDebug() << "invalid pack, remove invalid data, size:" << i;
                    buffetMutex.lock();
                    buf.remove(0, i);
                    buffetMutex.unlock();
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
            qDebug() << "invalid pack, remove all:" << size;
            buffetMutex.lock();
            buf.remove(0, size);
            buffetMutex.unlock();
        }

        requestFrame();
        return;
    }

//    qDebug() << buf.size() << header.bufferLength;
    if( buf.size() >= header.bufferLength )
    {
        // 热点状态更新
        if( cfg.hotspotEnable != header.hotspotState ) {
            cfg.hotspotEnable = header.hotspotState;
            emit f->hotspotEnableChanged();
        }

        if( cfg.frameMode != header.frameMode ) {
            cfg.frameMode = header.frameMode;
            cfg.frameFormat = header.frameFormat;
            emit f->frameModeChanged();
        }

        // 红外线状态更新
        if( cfg.infraredState != header.infraredState ) {
            cfg.infraredState = header.infraredState;
            emit f->infraredStateChanged();
        }

        cfg.devType = header.devType;
        cfg.width = header.width;
        cfg.height = header.height;
        cfg.version = QString::fromLocal8Bit(header.version);

//        qDebug() << header.devType << header.width << header.height << header.frameMode << header.frameFormat;

        QImage *image = f->rgb();
        CameraPixelFormat format = toCameraPixel(static_cast<hs::PixelFormat>(header.pixelFormat));
        if( format != __pix_invalid ) {
            if( header.devType == hs::__t2l_a4l ) {
                decode(reinterpret_cast<uint8_t *>(buf.data() + headerSize),
                       header.width, header.height,
                       format,
                       image,
                       (hs::FrameFormat)header.frameFormat);
            }
            else if( header.devType == hs::__tiny_1b ) {
                decode_tiny(reinterpret_cast<uint8_t *>(buf.data() + headerSize),
                            header.width, header.height,
                            image,
                            (hs::FrameFormat)header.frameFormat);
            }
        }
        else {
            qDebug() << "decode invalid pixel format";
        }

        buffetMutex.lock();
        buf.remove(0, header.bufferLength);
        buffetMutex.unlock();

//        qDebug() << QDateTime::currentMSecsSinceEpoch() - header.timestamp << "buf size:" << buf.size();

        // request next frame
        // 一秒25帧

//        int waitMs = 1000 / 25;
//        while ((QDateTime::currentMSecsSinceEpoch() - header.timestamp) < waitMs) {
//            std::this_thread::sleep_for(std::chrono::milliseconds(5));
//        }
////        qDebug() << (QDateTime::currentMSecsSinceEpoch() - header.timestamp);
//        requestFrame();

        QTimer::singleShot(40, Qt::PreciseTimer, [=](){
//            qint64 msec = QDateTime::currentMSecsSinceEpoch();
//            qDebug() << msec - header.timestamp;
            requestFrame();
        });
    }
}

CameraPixelFormat TcpCameraPrivate::toCameraPixel(const hs::PixelFormat &format)
{
    switch (format) {
    case hs::__yuyv: { return CameraPixelFormat::__pix_yuyv; }
    case hs::__xtherm_n16: { return CameraPixelFormat::__pix_custom; }
    case hs::__yuv420: { return CameraPixelFormat::__pix_yuv420p; }
    default: { return CameraPixelFormat::__pix_invalid; }
    }
}

void TcpCameraPrivate::decode(uint8_t *buf,
                              const int &width, const int &height,
                              const CameraPixelFormat &format, QImage *image,
                              const hs::FrameFormat &frameFormat)
{
    sendOvertimeCount = 0;

#ifdef TEMPERATURE_SDK
    if( temperatureData == NULL ) {
        temperatureData = (float*)calloc(width * (height - IMAGE_Y_OFFSET) + 10, sizeof(float));
    }
#endif

    frameCount ++;
    fpsCount ++;
    if( lastTimeStamp == 0 ) {
        lastTimeStamp = QDateTime::currentMSecsSinceEpoch();
    }
    else if( (QDateTime::currentMSecsSinceEpoch() - lastTimeStamp) >= 1000 ) {
        lastTimeStamp = 0;
        fps = fpsCount;
        fpsCount = 0;
    }

    if( frameBuffer.isNull() ) {
        if( format == __pix_custom ) {
            qDebug() << "n16 pixel format:" << width << height;
            frameBuffer.reset(new uint8_t[width * height * 2]);
        }
        else {
            qDebug() << "frame buffer size:" << ProviderCamera::byteSize(width, height, format) << width << height;
            frameBuffer.reset(new uint8_t[ProviderCamera::byteSize(width, height, format)]);
        }
    }

    if( frameFormat < hs::__full_frame ) {
        int frame_width = width;
        int frame_height = (height - IMAGE_Y_OFFSET) / 2;
        for(int i = 0; i < frame_height; i ++) {
           for(int j = 0; j < frame_width; j ++) {
               frameBuffer.data()[(2 * i + frameFormat) * frame_width + j] = buf[i * frame_width + j];
           }
        }

        int frame_data_length = frame_width * frame_height;
        int four_line_offset = frame_data_length * 4;
        memcpy(frameBuffer.data() + four_line_offset,
               buf + frame_data_length,
               width * 2 * IMAGE_Y_OFFSET);
    }

    uint16_t *data = reinterpret_cast<uint16_t *>((frameFormat == hs::__full_frame) ? buf : frameBuffer.data());
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
    float floatFpaTmp;

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

    if( (emiss != cfg.emiss)
            || (Refltmp != cfg.reflected)
            || (Airtmp != cfg.ambient)
            || (humi != cfg.humidness)
            || (distance != cfg.distance)
            || (correction != cfg.correction) ) {
        cfg.emiss = emiss;
        cfg.reflected = Refltmp;
        cfg.ambient = Airtmp;
        cfg.humidness = humi;
        cfg.distance = distance;
        cfg.correction = correction;
        emit f->cameraParamChanged();
    }

//    qDebug() << correction << Refltmp << Airtmp << humi << emiss << distance;

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
                      format == __pix_custom ? N16_MODE : YUYV_MODE);

#endif



    if( cameraSN.isEmpty() ) {
        cameraSN = QString(sn);
        emit f->productInfoChanged();
    }
    else if( lastTimeStamp == 0 ) {
        cameraSN = QString(sn);
        emit f->productInfoChanged();
    }

    uint8_t *bit = image->bits();
    if( format == __pix_yuyv ) {
        if( (frameFormat < hs::__full_frame) && (cfg.frameMode == hs::__fps_first) ) {
            y2_to_rgb24(frameBuffer.data(),
                        bit,
                        width, height - IMAGE_Y_OFFSET);
        }
        else if( (frameFormat == hs::__full_frame) && (cfg.frameMode == hs::__image_first) ) {
            nuc16_to_rgb24(data,
                           bit, width, height - IMAGE_Y_OFFSET);
        }
        else {
            f->pixelOperations()->yuv422_to_rgb(buf,
                                                bit,
                                                width, height - IMAGE_Y_OFFSET);
        }
    }
    else if( format == __pix_custom ) {
        nuc16_to_rgb24(data,
                       bit, width, height - IMAGE_Y_OFFSET);
    }

    int rotation = f->rotationIndex();
    QImage i = image->mirrored(rotation & 0x01, (rotation >> 1) & 0x01);

    QPainter pr(&i);
    QFontMetrics metrics(pr.font());

    // 帧率
    pr.setPen(Qt::white);
    QString str = QString::number(fps, 'f', 2);
    int fontW = metrics.horizontalAdvance(str);
    int fontH = metrics.height();
    pr.drawText(10, 10,
                fontW, fontH,
                Qt::AlignCenter,
                str);

#ifdef TEMPERATURE_SDK
    // 画温度信息
    if( showTemp ) {

//        QString strData;
//        for(int i = 0; i < 20 ; i++) {
//            strData += (QString::number((int)(temp[i] & 0xff))
//                        + QString(" , ")
//                        + QString::number((int)((temp[i] >> 8) & 0xff))
//                        + QString(", "));
//        }
//        qDebug() << strData
//                 << temperatureData[0]
//                 << temperatureData[1]
//                 << temperatureData[2]
//                 << temperatureData[3]
//                 << temperatureData[4]
//                 << temperatureData[5]
//                 << temperatureData[6]
//                 << float2byte(temperatureData[0]) << " , "
//                 << float2byte(temperatureData[3]) << " , "
//                 << float2byte(temperatureData[6]);


        // 最大温度
        pr.setPen(Qt::red);
        QPoint pos = f->rotatePoint(QPoint(temperatureData[1], temperatureData[2]));
        pr.drawLine(pos.x() - 10, pos.y(),
                    pos.x() + 10, pos.y());
        pr.drawLine(pos.x(), pos.y() - 10,
                    pos.x(), pos.y() + 10);

        QString str = QString::number(temperatureData[3], 'f', 1);
        int fontW = metrics.horizontalAdvance(str);
        int fontH = metrics.height();
        pr.drawText(pos.x(), pos.y() - fontH,
                    fontW, fontH,
                    Qt::AlignCenter,
                    str);

        // 最小温度
        pr.setPen(Qt::blue);
        pos = f->rotatePoint(QPoint(temperatureData[4], temperatureData[5]));
        pr.drawLine(pos.x() - 10, pos.y(),
                    pos.x() + 10, pos.y());
        pr.drawLine(pos.x(), pos.y() - 10,
                    pos.x(), pos.y() + 10);

        str = QString::number(temperatureData[6], 'f', 1);
        fontW = metrics.horizontalAdvance(str);
        fontH = metrics.height();
        pr.drawText(pos.x(), pos.y() - fontH,
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
    }
#endif
    if( encode->status() ) {
        encode->pushEncode(i.copy());
    }

    f->setUrlImage(i);
}

void TcpCameraPrivate::decode_tiny(uint8_t *buf,
                                   const int &width, const int &height,
                                   QImage *image,
                                   const hs::FrameFormat &frameFormat)
{
    if( frameBuffer.isNull() ) {
        frameBuffer.reset(new uint8_t[width * height / 2]);
    }

    if( frameFormat < hs::__full_frame ) {
        for(int i = 0; i < height / 4; i ++) {
            memcpy(frameBuffer.data() + ((i * 2 + frameFormat) * width),
                   buf + (i * width),
                   width);
        }

        f->pixelOperations()->gray_to_pseudo(frameBuffer.data(),
                                             image->bits(),
                                             width, height / 2);
        int rotation = f->rotationIndex();
        QImage i = image->mirrored(rotation & 0x01, (rotation >> 1) & 0x01);
        f->setUrlImage(i);
    }
}

void TcpCameraPrivate::nuc16_to_rgb24(const uint16_t *nuc, uint8_t *rgb, const int &width, const int &height)
{
    uint index = width * height;
    ushort detectAvg = nuc[index ++];
    ushort fpaTmp = nuc[index ++];
    ushort maxx1 = nuc[index ++];
    ushort maxy1 = nuc[index ++];
    ushort max = nuc[index ++];
    ushort minx1 = nuc[index ++];
    ushort miny1 = nuc[index ++];
    ushort min = nuc[index ++];
    ushort avg = nuc[index];

    int avgSubMin = (avg - min) > 0 ? (avg - min) : 1;
    int maxSubAvg = (max - avg) > 0 ? (max - avg) : 1;
    int ro1 = (avg - min) > 97 ? 97 : (avg - min);
    int ro2 = (max - avg) > 157 ? 157 : (max - avg);
    uint8_t gray = 0;
    for(int i = 0; i < height; i ++)
    {
        for(int j = 0; j < width; j ++)
        {
            index = i * width + j;
            if( nuc[index] > avg ) {
                gray = (int)(ro2 * (nuc[index] - avg) / maxSubAvg + 97);
            }
            else {
                gray = (int)(ro1 * (nuc[index] - avg) / avgSubMin + 97);
            }

            RGBPixel pixel;
            f->pixelOperations()->gray_to_rgb_pixel(gray, &pixel);
            *rgb ++ = pixel.r;
            *rgb ++ = pixel.g;
            *rgb ++ = pixel.b;
        }
    }
}

void TcpCameraPrivate::y2_to_rgb24(const uint8_t *in, uint8_t *out, const int &width, const int &height)
{
    for(int i = 0; i < height; i ++) {
        for(int j = 0; j < width; j ++) {
            RGBPixel pixel;

//            uint8_t gray = in[i * width + j];
//            if( gray < 127 ) {
//                gray = gray >> 1;
//            }
//            pixelOperations->gray_to_rgb_pixel(gray, &pixel);

            f->pixelOperations()->gray_to_rgb_pixel(in[i * width + j], &pixel);

            *out ++ = pixel.r;
            *out ++ = pixel.g;
            *out ++ = pixel.b;
        }
    }
}

void TcpCameraPrivate::readSetting()
{
//    deviceIP = g_Config->readSetting("Tcp/DevceIP", "192.168.1.1").toString();
    cfg.ip = SERVER_IP;
    cfg.port = SERVER_PORT;
    cfg.palette = g_Config->readSetting("Tcp/palette", 0).toUInt();
    cfg.emiss = g_Config->readSetting("Tcp/emiss", 1.0).toDouble();
    cfg.reflected = g_Config->readSetting("Tcp/reflected", 1.0).toDouble();
    cfg.ambient = g_Config->readSetting("Tcp/ambient", 1.0).toDouble();
    cfg.humidness = g_Config->readSetting("Tcp/humidness", 1.0).toDouble();
    cfg.correction = g_Config->readSetting("Tcp/correction", 1.0).toDouble();
    cfg.distance = g_Config->readSetting("Tcp/distance", 0).toUInt();
    cfg.frameMode = g_Config->readSetting("Tcp/frameMode", hs::__image_first).toUInt();
    showTemp = g_Config->readSetting("Tcp/showtemp", false).toBool();
    cfg.hotspotSsid = g_Config->readSetting("Tcp/hotspotSsid", "").toString();
    cfg.hotspotPassword = g_Config->readSetting("Tcp/hotspotPassword", "").toString();
    cfg.hotspotEnable = false;
    cfg.infraredState = hs::__infrared_off;
}

void TcpCameraPrivate::saveSetting()
{
    QVector<SettingParam> param;
//    param.append({"Tcp/DeviceIP", deviceIP});

    param.append({"Tcp/palette", cfg.palette});
    param.append({"Tcp/emiss", cfg.emiss});
    param.append({"Tcp/reflected", cfg.reflected});
    param.append({"Tcp/ambient", cfg.ambient});
    param.append({"Tcp/humidness", cfg.humidness});
    param.append({"Tcp/correction", cfg.correction});
    param.append({"Tcp/distance", cfg.distance});
    param.append({"Tcp/frameMode", cfg.frameMode});
    param.append({"Tcp/showtemp", showTemp});
    param.append({"Tcp/hotspotSsid", cfg.hotspotSsid});
    param.append({"Tcp/hotspotPassword", cfg.hotspotPassword});
    g_Config->insertSetting(param);
}

void TcpCameraPrivate::capture()
{
    if( !captureThread.joinable() ) {
        captureThread = std::thread([=](){
            QImage *rgb = f->rgb();
            QImage image = rgb->copy();
            image.setText(QString("IMG_TYPE"), QString("IMAGE"));
            QPixmap pix = QPixmap::fromImage(image);
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

            // save frame buffer
            /*
            fileName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + QString(".dat");
            QFile file(g_Config->imageGalleryPath() + fileName);
            bool flag = file.open(QIODevice::WriteOnly);
            if( flag ) {
                qDebug() << "save frame path:" << file.fileName();
                QDataStream st(&file);
                frameBufferMutex.lock();
                st.writeRawData(reinterpret_cast<char *>(frameBuffer.data()),
                                cfg.header.width * cfg.header.height * 2);
                frameBufferMutex.unlock();
                file.close();
            }
            else {
                emit f->msg(QString("Save frame data fail"));
            }
            */
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
    hs::t_packet p = packet(hs::__req_shutter);
    emit write(requestByte(&p));
}

void TcpCameraPrivate::setPalette(const int &index)
{
    qDebug() << "set palette:" << index;
    cfg.palette = index;
    f->pixelOperations()->updatePseudoColor((PseudoColorTable)index);
    emit f->paletteChanged();
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

void TcpCameraPrivate::setFrameMode(const int &mode)
{
    if( f->isConnected() ) {
        hs::t_packet p = packet(hs::__req_frame_mode);
        p.frameMode = mode;
        emit write(requestByte(&p));
    }
    else {
        cfg.frameMode = mode;
    }
}

void TcpCameraPrivate::setCameraParam(const qreal &emiss, const qreal &reflected,
                                      const qreal &ambient, const qreal &humidness,
                                      const qreal &correction, const int &distance)
{
    cfg.emiss = emiss;
    cfg.reflected = reflected;
    cfg.ambient = ambient;
    cfg.humidness = humidness;
    cfg.correction = correction;
    cfg.distance = distance;
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
//    QByteArray byte = requestByte(hs::__req_camera_param);
    hs::t_packet p = packet(hs::__req_camera_param);
    p.emiss = emiss;
    p.reflected = reflected;
    p.ambient = ambient;
    p.humidness = humidness;
    p.correction = correction;
    p.distance = distance;
    emit write(requestByte(&p));
}

bool TcpCameraPrivate::setHotspotParam(const QString &ssid, const QString &password)
{
    if( ssid.toUtf8().size() >= HOTSPOT_SSID_LENGTH ) {
        emit f->msg(tr("SSID length limit exceeded"));
        return false;
    }
    if( password.size() >= HOTSPOT_PASSWORD_LENGTH ) {
        emit f->msg(tr("Password length limit exceeded"));
        return false;
    }
    if( password.size() < 8 ) {
        emit f->msg(tr("Password minimum length is 8"));
        return false;
    }

    if( !ssid.isEmpty() && !password.isEmpty() ) {
        cfg.hotspotSsid = ssid;
        cfg.hotspotPassword = password;

        if( f->isConnected() ) {
            hs::t_packet p = packet(hs::__req_hotspot_info);
            QByteArray _ssid = ssid.toUtf8();
            QByteArray _password = password.toUtf8();
            memcpy(p.hotspotSSID, _ssid.data(), _ssid.size());
            p.hotspotSSID[_ssid.size()] = '\0';
            memcpy(p.hotspotPassword, _password.data(), _password.size());
            p.hotspotPassword[_password.size()] = '\0';
            emit write(requestByte(&p));
        }
        else {
            emit f->msg(tr("Device not connect"));
        }
    }
    return true;
}

bool TcpCameraPrivate::isValidIpv4Addres(const QString &ip)
{
    if( ip.isEmpty() ) {
        return false;
    }

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

void TcpCameraPrivate::openServer()
{

    server.reset(new QTcpServer);
    QObject::connect(server.data(), &QTcpServer::newConnection, [=](){
        QTcpSocket *s = server->nextPendingConnection();
        if( s != nullptr ) {
            clearSearcher();

            socket.reset(s);

            QString ipv6 = socket->peerAddress().toString();
            int index = ipv6.lastIndexOf(':');
            QString ipv4 = ipv6.mid(index + 1, ipv6.length() - index);
            qDebug() << QString("client connected, ip: %1").arg(ipv4);

            socketConnection();
        }
    });
    bool state = server->listen(QHostAddress::Any, 8080);
    qDebug() << "server listen state:" << state;

}

void TcpCameraPrivate::closeServer()
{
    if( !server.isNull() ) {
        server->close();
        server.reset();
    }
}

#ifdef Q_OS_ANDROID
void TcpCameraPrivate::UvcCallback(void *content, void *data, const int &width, const int &height, const int &bufferLength)
{
    TcpCameraPrivate *cam = static_cast<TcpCameraPrivate *>(content);

//    CameraPixelFormat format = cam->uvc->pixelFormat();
    QImage *rgb = cam->f->rgb();

    cam->decode(reinterpret_cast<uint8_t *>(data),
                width, height,
                __pix_custom,
                rgb,
                hs::__full_frame);

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
