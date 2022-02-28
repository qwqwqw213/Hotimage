#ifndef IOSINTERFACE_H
#define IOSINTERFACE_H

#include "QObject"

typedef struct {
    int top;
    int left;
    int bottom;
    int right;
} SafeArea;

class IOSInterface : public QObject
{
    Q_OBJECT
public:
    explicit IOSInterface(QObject *parent = nullptr);
    ~IOSInterface();

    Q_PROPERTY(bool hotspotOpen READ hotspotOpen NOTIFY updateHotspotInfo)
    bool hotspotOpen();
    Q_PROPERTY(QString hotspotSSID READ hotspotSSID NOTIFY updateHotspotInfo)
    QString hotspotSSID();
    Q_PROPERTY(QString hotspotPassword READ hotspotPassword NOTIFY updateHotspotInfo)
    QString hotspotPassword();

    // 打开手机热点
    Q_INVOKABLE void openHotspot();

    void keepScreenOn();
    void safeArea(SafeArea *t);

Q_SIGNALS:
    void updateHotspotInfo();
};

#endif // IOSINTERFACE_H
