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
#define UVC_MODE                5

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
    QByteArray requestByte(const hs::RequestType &type);
    void requestFrame(const hs::RequestType &type = hs::__req_frame);

    QScopedPointer<QTcpSocket> socket;
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
    int frameMode;

    QString localIP;
    QString deviceIP;

    QString cameraSN;

    uint8_t *frameBuffer;
    CameraPixelFormat toCameraPixel(const hs::PixelFormat &format);
    void decode(uint8_t *buf,
                const int &width, const int &height,
                const CameraPixelFormat &format,
                QImage *image,
                const bool &fullBuffer);
    void nuc16_to_rgb24(const uint16_t *nuc, uint8_t *rgb, const int &width, const int &height);

    QScopedPointer<PixelOperations> pixelOperations;

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
    return p->uvc.isNull() ? (isOpen() ? (!p->socket.isNull()) : false) : p->uvc->isOpen();
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
    p->exit = 1;
    if( isOpen() ) {
        if( encoding() ) {
            closeRecord();
        }
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

int TcpCamera::frameMode()
{
    return (p->cfg.set.frameFormat == hs::__full_frame ? 1 : 0);
}

void TcpCamera::setFrameMode(const int &mode)
{
    p->cfg.set.frameFormat = (mode == 1 ? hs::__full_frame : hs::__even_frame);
    emit frameModeChanged();
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
    return QByteArray(p->cfg.header.version, 11);
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
    if( ssid.size() >= sizeof(p->cfg.set.hotspotSSID) ) {
        emit msg(tr("SSID length limit exceeded"));
        return false;
    }
    if( password.size() >= sizeof(p->cfg.set.hotspotPassword) ) {
        emit msg(tr("Password length limit exceeded"));
        return false;
    }
    if( password.size() < 8 ) {
        emit msg(tr("Password minimum length is 8"));
        return false;
    }

    if( !ssid.isEmpty() && !password.isEmpty() ) {
        p->cfg.hotspotSsid = ssid;
        p->cfg.hotspotPassword = password;

        QByteArray _ssid = ssid.toLatin1();
        QByteArray _password = password.toLatin1();
        memcpy(p->cfg.set.hotspotSSID, _ssid.data(), _ssid.length());
        p->cfg.set.hotspotSSID[_ssid.length()] = '\0';
        memcpy(p->cfg.set.hotspotPassword, _password.data(), _password.length());
        p->cfg.set.hotspotPassword[_password.length()] = '\0';

        QByteArray byte(p->requestByte(hs::__req_hotspot_info));
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
        g_Config->appendLog(QString("camera state update: %1").arg(p->uvc->isOpen()));
        if( p->uvc->isOpen() ) {
            p->uvc->zoomAbsolute(0x8000 | UVC_MODE);
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

    frameBuffer = nullptr;

    thread = new QThread;
    QObject::connect(thread, &QThread::started, [=](){
        exit = 0;
        buf.clear();

        sendTimer = new QTimer;
        sendTimer->setInterval(2000);
        sendTimer->setSingleShot(true);

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

        QObject::connect(sendTimer, &QTimer::timeout, [=](){
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

        g_Config->appendLog(QString("start search socket"));
        searchTimer = new QTimer;
        searchTimer->setTimerType(Qt::PreciseTimer);
        QObject::connect(searchTimer, &QTimer::timeout, this, &TcpCameraPrivate::searchOvertime);
        qDebug() << searchTimer->timerType();

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
    });

    QObject::connect(thread, &QThread::finished, [=](){
        clearSearcher();

        sendTimer->stop();
        sendTimer->deleteLater();
        sendTimer = nullptr;

        if( !socket.isNull() ) {
            QByteArray byte = requestByte(hs::__req_disconnect);
            socket->write(byte);
            bool flag = socket->waitForBytesWritten(10 * 1000);
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

        if( frameBuffer ) {
            delete [] frameBuffer;
            frameBuffer = nullptr;
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

QByteArray TcpCameraPrivate::requestByte(const hs::RequestType &type)
{
    QByteArray byte;
    byte.resize(HANDSHAKE_PACK_SIZE);
    cfg.set.marker[0] = 'R';
    cfg.set.marker[1] = 'e';
    cfg.set.marker[2] = 'C';
    cfg.set.marker[3] = 'v';
    cfg.set.request = type;
    if( cfg.set.frameFormat != hs::__full_frame ) {
        cfg.set.frameFormat = !cfg.set.frameFormat;
    }
//    qDebug() << "req type:" << cfg.set.request;
    memcpy(byte.data(), &cfg.set, sizeof (hs::t_packet));
    return byte;
}

void TcpCameraPrivate::requestFrame(const hs::RequestType &type)
{
    if( exit == 1 ) {
        return;
    }
    sendTimer->start();
//    qDebug() << "req frame type:" << type;
    emit write(requestByte(type));
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

        if( state == QAbstractSocket::ConnectedState ) {
            // 异常断开, 重新请求
            requestFrame(hs::__req_init);
        }
    }, Qt::QueuedConnection);

    QObject::connect(socket.data(), &QTcpSocket::disconnected, this, [=](){
        if( !socket.isNull() ) {
            qDebug() << "socket disconnect:" << socket->state();
            emit f->connectStatusChanged();
            unpackMutex.lock();
            buf.clear();
            unpackMutex.unlock();

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
    while ((exit != 1) && !socket.isNull() && (time < 30)) {
        socket->connectToHost(dev, cfg.port);
        socket->waitForConnected(3000);
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
    if( manualConnState ) {
        return;
    }
    clearSearcher();

    if( socket.isNull() ) {
        qDebug() << "search socket overtime";
        searchDevice();
    }
}

static qint64 msecsSinceEpoch = 0;
void TcpCameraPrivate::onReadyRead()
{
    if( sendTimer->isActive() ) {
        sendTimer->stop();
    }

    unpackMutex.lock();
    buf.push_back(socket->readAll());
    unpackMutex.unlock();

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
            qDebug() << "invalid pack, remove all:" << size;
            unpackMutex.lock();
            buf.remove(0, size);
            unpackMutex.unlock();
        }
        return;
    }

//    qDebug() << buf.size() << header.bufferLength;
    if( buf.size() >= header.bufferLength )
    {
        // 热点状态更新
        if( cfg.hotspotEnable != header.hotspot ) {
            cfg.hotspotEnable = header.hotspot;
            emit f->hotspotEnableChanged();
        }

        // 0x8005 模式 调色盘更新
        if( (cfg.set.cameraMode == 5) && (cfg.set.palette != header.palette) ) {
            cfg.set.palette = header.palette;
            emit f->paletteChanged();
        }

        memcpy(&cfg.header, &header, sizeof (hs::t_header));

        QImage *image = f->rgb();
        CameraPixelFormat format = toCameraPixel(static_cast<hs::PixelFormat>(header.pixelFormat));
        if( format != __pix_invalid ) {
            decode(reinterpret_cast<uint8_t *>(buf.data() + headerSize),
                   cfg.header.width, cfg.header.height,
                   format,
                   image,
                   header.frameFormat == hs::__full_frame);
        }
        else {
            qDebug() << "decode invalid pixel format";
        }

        unpackMutex.lock();
        buf.remove(0, header.bufferLength);
        unpackMutex.unlock();

        // request next frame
        if( msecsSinceEpoch == 0 ) {
            msecsSinceEpoch = QDateTime::currentMSecsSinceEpoch();
        }

        int wait = header.frameFormat == hs::__full_frame ? 40 : 25;
        while ((QDateTime::currentMSecsSinceEpoch() - msecsSinceEpoch) < wait) {
        }
        msecsSinceEpoch = QDateTime::currentMSecsSinceEpoch();
        requestFrame();

//        QTimer::singleShot(header.frameFormat == hs::__full_frame ?
//                               40 : 25, this, [=](){
//            qint64 msec = QDateTime::currentMSecsSinceEpoch();
//            qDebug() << "interval:" << (msec - msecsSinceEpoch);
//            msecsSinceEpoch = msec;
//            requestFrame();
//        });
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
                              const CameraPixelFormat &format, QImage *image, const bool &fullBuffer)
{
    sendOvertimeCount = 0;

#ifdef TEMPERATURE_SDK
    if( temperatureData == NULL ) {
        temperatureData = (float*)calloc(width * (height - IMAGE_Y_OFFSET), sizeof(float));
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

    if( !fullBuffer ) {
        if( frameBuffer == nullptr ) {
            if( format == __pix_custom ) {
                qDebug() << "n16 pixel format:" << width << height;
                frameBuffer = new uint8_t[width * height * 2];
            }
            else {
                qDebug() << "frame buffer size:" << ProviderCamera::byteSize(width, height, format) << width << height;
                frameBuffer = new uint8_t[ProviderCamera::byteSize(width, height, format)];
            }
        }

//        qDebug() << "frame index:" << cfg.header.frameFormat;
        int packIndex = cfg.header.frameFormat;
        int row = width * 2;
        for(int i = 0; i < height / 2; i ++) {
            memcpy(frameBuffer + ((2 * i + packIndex) * row),
                   buf + (i * row),
                   row);
        }
    }

    uint16_t *data = fullBuffer ?
                reinterpret_cast<uint16_t *>(buf) : reinterpret_cast<uint16_t *>(frameBuffer);
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
        emit f->connectStatusChanged();
        emit f->cameraParamChanged();
    }
    else {
        cameraSN = QString(sn);
        emit f->productInfoChanged();
    }

    uint8_t *bit = image->bits();
    if( format == __pix_yuyv ) {
        PixelOperations::yuv422_to_rgb(fullBuffer ? buf : frameBuffer,
                                       bit,
                                       width, height - IMAGE_Y_OFFSET);
    }
    else if( format == __pix_custom ) {
        nuc16_to_rgb24(data,
                       bit, width, height - IMAGE_Y_OFFSET);
    }
    else if( format == __pix_yuv420p ) {
        PixelOperations::yuv420p_to_rgb(fullBuffer ? buf : frameBuffer,
                                        bit,
                                        width, height - IMAGE_Y_OFFSET);
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
            pixelOperations->gray_to_rgb_pixel(gray, &pixel);
            *rgb ++ = pixel.r;
            *rgb ++ = pixel.g;
            *rgb ++ = pixel.b;
        }
    }
}

void TcpCameraPrivate::readSetting()
{
//    deviceIP = g_Config->readSetting("Tcp/DevceIP", "192.168.1.1").toString();
    cfg.ip = SERVER_IP;
    cfg.port = SERVER_PORT;
    cfg.set.cameraMode = g_Config->readSetting("Tcp/cameraMode", UVC_MODE).toUInt();
    cfg.set.palette = g_Config->readSetting("Tcp/palette", 0).toUInt();
    cfg.set.emiss = g_Config->readSetting("Tcp/emiss", 1.0).toDouble();
    cfg.set.reflected = g_Config->readSetting("Tcp/reflected", 1.0).toDouble();
    cfg.set.ambient = g_Config->readSetting("Tcp/ambient", 1.0).toDouble();
    cfg.set.humidness = g_Config->readSetting("Tcp/humidness", 1.0).toDouble();
    cfg.set.correction = g_Config->readSetting("Tcp/correction", 1.0).toDouble();
    cfg.set.distance = g_Config->readSetting("Tcp/distance", 0).toUInt();
    cfg.set.frameFormat = g_Config->readSetting("Tcp/frameFormat", 0).toUInt();
    if( cfg.set.frameFormat != hs::__full_frame ) {
        cfg.set.frameFormat = hs::__even_frame;
    }
    showTemp = g_Config->readSetting("Tcp/showtemp", false).toBool();
    cfg.hotspotSsid = g_Config->readSetting("Tcp/hotspotSsid", "").toString();
    cfg.hotspotPassword = g_Config->readSetting("Tcp/hotspotPassword", "").toString();
    cfg.hotspotEnable = false;

    QByteArray _ssid = cfg.hotspotSsid.toLatin1();
    QByteArray _password = cfg.hotspotPassword.toLatin1();
    memcpy(cfg.set.hotspotSSID, _ssid.data(), _ssid.length());
    cfg.set.hotspotSSID[_ssid.length()] = '\0';
    memcpy(cfg.set.hotspotPassword, _password.data(), _password.length());
    cfg.set.hotspotPassword[_password.length()] = '\0';

    if( cfg.set.palette < __Pseudo_WhiteHot
            || cfg.set.palette > __InvalidPseudoTable ) {
        qDebug() << "invalid palette index:" << cfg.set.palette;
        cfg.set.palette = __Pseudo_WhiteHot;
    }
    pixelOperations.reset(new PixelOperations);
    pixelOperations->initializerPseudo((PseudoColorTable)cfg.set.palette);
}

void TcpCameraPrivate::saveSetting()
{
    QVector<SettingParam> param;
//    param.append({"Tcp/DeviceIP", deviceIP});

    param.append({"Tcp/cameraMode", cfg.set.cameraMode});
    param.append({"Tcp/palette", cfg.set.palette});
    param.append({"Tcp/emiss", cfg.set.emiss});
    param.append({"Tcp/reflected", cfg.set.reflected});
    param.append({"Tcp/ambient", cfg.set.ambient});
    param.append({"Tcp/humidness", cfg.set.humidness});
    param.append({"Tcp/correction", cfg.set.correction});
    param.append({"Tcp/distance", cfg.set.distance});
    param.append({"Tcp/frameFormat", cfg.set.frameFormat});
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
    QByteArray byte = requestByte(hs::__req_shutter);
    emit write(byte);
}

void TcpCameraPrivate::setPalette(const int &index)
{
    qDebug() << "set palette:" << index;
    cfg.set.palette = index;
#ifdef Q_OS_ANDROID
    if( !uvc.isNull() ) {
        if( UVC_MODE == 4 ) {
            pixelOperations->updatePseudoColor((PseudoColorTable)index);
        }
        else {
            uvc->zoomAbsolute(0x8800 | (index & 0xfff));
        }
        emit f->paletteChanged();
        return;
    }
#endif
    if( cfg.set.cameraMode == 4 ) {
        pixelOperations->updatePseudoColor((PseudoColorTable)index);
    }
    else {
        emit write(requestByte(hs::__req_palette));
    }
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
    QByteArray byte = requestByte(hs::__req_camera_param);
    emit write(byte);
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

#ifdef Q_OS_ANDROID
void TcpCameraPrivate::UvcCallback(void *content, void *data, const int &width, const int &height, const int &bufferLength)
{
    TcpCameraPrivate *cam = static_cast<TcpCameraPrivate *>(content);

//    CameraPixelFormat format = cam->uvc->pixelFormat();
    QImage *rgb = cam->f->rgb();

    cam->decode(reinterpret_cast<uint8_t *>(data),
                width, height,
                UVC_MODE == 4 ? __pix_custom : __pix_yuyv, rgb, true);

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
