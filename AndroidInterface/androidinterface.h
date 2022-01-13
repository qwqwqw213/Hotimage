#ifndef ANDROIDINTERFACE_H
#define ANDROIDINTERFACE_H

 class AndroidInterface;

#ifdef g_Android
#undef g_Android
#endif
#define g_Android       (static_cast<AndroidInterface *>(AndroidInterface::androidInterface()))


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

private:
    friend class AndroidInterfacePrivate;
    QScopedPointer<AndroidInterfacePrivate> p;

    static AndroidInterface *android_self;
};

#endif // ANDROIDINTERFACE_H
