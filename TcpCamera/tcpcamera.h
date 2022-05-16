#ifndef RECV_H
#define RECV_H

#include "tcpdef.h"
#include "providercamera.hpp"

#include "QPixmap"
#include "QImage"
#include "QHostInfo"
#include "QHostAddress"
#include "QTcpSocket"
#include "QQuickImageProvider"

class TcpCameraPrivate;
class TcpCamera : public ProviderCamera
{
    Q_OBJECT

public:
    explicit TcpCamera(QObject *parent = nullptr);
    ~TcpCamera();

    static TcpCamera *instance();

    bool isOpen() override;
    int width() override;
    int height() override;
    int fps() override;

    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectStatusChanged)
    bool isConnected();
    Q_PROPERTY(QString localIp READ localIp NOTIFY connectStatusChanged)
    QString localIp();
    Q_PROPERTY(QString deviceIp READ deviceIp NOTIFY connectStatusChanged)
    QString deviceIp();

    Q_PROPERTY(bool manaulConnectState READ manaulConnectState NOTIFY manualConnectStateChanged)
    bool manaulConnectState();
    Q_INVOKABLE void manualConnect(const QString &devIp);

    void open();
//    void open(tcp_config *s);
    void close();

    Q_INVOKABLE void capture();

    Q_INVOKABLE void shutter();
    Q_INVOKABLE void setPalette(const int &index);
    Q_INVOKABLE void setCameraParam(const qreal &emiss, const qreal &reflected,
                                    const qreal &ambient, const qreal &humidness,
                                    const qreal &correction, const int &distance);

    Q_PROPERTY(int palette READ palette NOTIFY paletteChanged)
    int palette();

    Q_PROPERTY(qreal emiss READ emiss NOTIFY cameraParamChanged)
    qreal emiss();
    Q_PROPERTY(qreal reflected READ reflected NOTIFY cameraParamChanged)
    qreal reflected();
    Q_PROPERTY(qreal ambient READ ambient NOTIFY cameraParamChanged)
    qreal ambient();
    Q_PROPERTY(qreal humidness READ humidness NOTIFY cameraParamChanged)
    qreal humidness();
    Q_PROPERTY(qreal correction READ correction NOTIFY cameraParamChanged)
    qreal correction();
    Q_PROPERTY(qreal distance READ distance NOTIFY cameraParamChanged)
    uint16_t distance();

    Q_PROPERTY(int frameMode READ frameMode WRITE setFrameMode NOTIFY frameModeChanged)
    int frameMode();
    // 0: half frame
    // 1: full frame
    void setFrameMode(const int &mode);

    Q_PROPERTY(bool showTemp READ showTemp WRITE setShowTemp NOTIFY showTempChanged)
    bool showTemp();
    void setShowTemp(const bool &show);

    Q_PROPERTY(bool encoding READ encoding NOTIFY encodingChanged)
    bool encoding();
    Q_INVOKABLE void openRecord();
    Q_INVOKABLE void closeRecord();

    Q_PROPERTY(QString recordTime READ recordTime NOTIFY recordTimeChanged)
    QString recordTime();

    Q_PROPERTY(QString cameraSN READ cameraSN NOTIFY cameraSNChanged)
    QString cameraSN();

    Q_PROPERTY(bool hotspotEnable READ hotspotEnable NOTIFY hotspotParamChanged)
    bool hotspotEnable();
    Q_PROPERTY(QString hotspotSSID READ hotspotSSID NOTIFY hotspotParamChanged)
    QString hotspotSSID();
    Q_PROPERTY(QString hotspotPassword READ hotspotPassword NOTIFY hotspotParamChanged)
    QString hotspotPassword();
    Q_INVOKABLE bool setHotspotParam(const QString &ssid, const QString &password);

    void saveSetting();

    void openUsbCamera(const int &fd);
    void closeUsbCamera();

private:
    friend class TcpCameraPrivate;
    QScopedPointer<TcpCameraPrivate> p;

Q_SIGNALS:
    void msg(const QString &str);
    void videoFrameChanged();
    void captureFinished(const QString &path);
    void connectStatusChanged();
    void paletteChanged();
    void showTempChanged();
    void encodingChanged();
    void recordTimeChanged();
    void videoFrame(QImage);
    void cameraSNChanged();
    void cameraParamChanged();
    void manualConnectStateChanged();
    void hotspotParamChanged();
    void frameModeChanged();
};

#endif // RECV_H
