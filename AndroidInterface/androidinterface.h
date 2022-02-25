#ifndef ANDROIDINTERFACE_H
#define ANDROIDINTERFACE_H

 class AndroidInterface;

#ifdef g_Android
#undef g_Android
#endif
#define g_Android       (static_cast<AndroidInterface *>(AndroidInterface::androidInterface()))
#define ISVALID(x)      (x != nullptr)


#include <QObject>

class AndroidInterfacePrivate;
class AndroidInterface : public QObject
{
    Q_OBJECT

public:
    explicit AndroidInterface(QObject *parent = nullptr);
    ~AndroidInterface();

    static AndroidInterface * androidInterface() {
        return android_self;
    }

    void updateSetting();

    enum RotationScreen
    {
        __unspecified = -1,      // 跟随系统
        __landscape,            // 横向
        __portrait,             // 纵向
    };
    // 屏幕旋转
    Q_INVOKABLE void setRotationScreen(const int &index);

    // 申请sd card读写权限
    Q_INVOKABLE void requestPhotoWritePermission();

    int safeAeraLeft();
    int safeAeraRight();
    int safeAreaTop();
    int safeAeraBottom();

    Q_PROPERTY(bool hotspotOpen READ hotspotOpen NOTIFY updateHotspotInfo)
    bool hotspotOpen();
    Q_PROPERTY(QString hotspotSSID READ hotspotSSID NOTIFY updateHotspotInfo)
    QString hotspotSSID();
    Q_PROPERTY(QString hotspotPassword READ hotspotPassword NOTIFY updateHotspotInfo)
    QString hotspotPassword();

    // 打开手机热点
    Q_INVOKABLE void openHotspot();

private:
    friend class AndroidInterfacePrivate;
    QScopedPointer<AndroidInterfacePrivate> p;

    static AndroidInterface *android_self;

Q_SIGNALS:
    void requestPermissionFail();

    void updateHotspotInfo();
};

#endif // ANDROIDINTERFACE_H
