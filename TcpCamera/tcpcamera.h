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

    bool isOpen();

    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectStatusChanged)
    bool isConnected();

    void open();
//    void open(tcp_config *s);
    void close();

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

    Q_PROPERTY(qreal emiss READ emiss CONSTANT)
    qreal emiss();
    Q_PROPERTY(qreal reflected READ reflected CONSTANT)
    qreal reflected();
    Q_PROPERTY(qreal ambient READ ambient CONSTANT)
    qreal ambient();
    Q_PROPERTY(qreal humidness READ humidness CONSTANT)
    qreal humidness();
    Q_PROPERTY(qreal correction READ correction CONSTANT)
    qreal correction();
    Q_PROPERTY(qreal distance READ distance CONSTANT)
    uint16_t distance();
    Q_PROPERTY(bool showTemp READ showTemp WRITE setShowTemp NOTIFY showTempChanged)
    bool showTemp();
    void setShowTemp(const bool &show);

    Q_PROPERTY(bool encoding READ encoding NOTIFY encodingChanged)
    bool encoding();
    Q_INVOKABLE void openRecode();

    Q_PROPERTY(QString recordTime READ recordTime NOTIFY recordTimeChanged)
    QString recordTime();

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
};

#endif // RECV_H
