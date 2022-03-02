#ifndef RECV_H
#define RECV_H

#include "tcpdef.h"
#include "ImageProvider/imageprovider.h"

#include <QObject>

#include "QPixmap"
#include "QImage"
#include "QHostInfo"
#include "QHostAddress"
#include "QTcpSocket"
#include "QQuickImageProvider"

class TcpCameraPrivate;
class TcpCamera : public QObject
{
    Q_OBJECT

public:
    explicit TcpCamera(QObject *parent = nullptr);
    ~TcpCamera();

    static TcpCamera *instance();

    bool isOpen();

    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectStatusChanged)
    bool isConnected();

    void open();
//    void open(tcp_config *s);
    void close();
    void exit();

    double fps();

    Q_INVOKABLE void capture();

    ImageProvider *provider();

    Q_PROPERTY(QString videoFrameUrl READ videoFrameUrl NOTIFY videoFrameChanged)
    QString videoFrameUrl();

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

    Q_PROPERTY(bool showTemp READ showTemp WRITE setShowTemp NOTIFY showTempChanged)
    bool showTemp();
    void setShowTemp(const bool &show);

    Q_PROPERTY(bool encoding READ encoding NOTIFY encodingChanged)
    bool encoding();
    Q_INVOKABLE void openRecord();

    Q_PROPERTY(QString recordTime READ recordTime NOTIFY recordTimeChanged)
    QString recordTime();

    Q_PROPERTY(QString cameraSN READ cameraSN NOTIFY cameraSNChanged)
    QString cameraSN();

    Q_PROPERTY(QString localIp READ localIp NOTIFY wirelessParamChanged)
    QString localIp();
    Q_PROPERTY(QString deviceIp READ deviceIp NOTIFY wirelessParamChanged)
    QString deviceIp();
    Q_PROPERTY(QString hotspotSSID READ hotspotSSID NOTIFY wirelessParamChanged)
    QString hotspotSSID();
    Q_PROPERTY(QString hotspotPassword READ hotspotPassword NOTIFY wirelessParamChanged)
    QString hotspotPassword();
    Q_INVOKABLE bool setWirelessParam(const QString &deviceIp, const QString &ssid, const QString &password);

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
    void wirelessParamChanged();
};

#endif // RECV_H
