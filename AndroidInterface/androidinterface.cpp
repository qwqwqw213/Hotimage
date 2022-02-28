#include "androidinterface.h"

#ifdef Q_OS_ANDROID
#include "QAndroidJniObject"
#include "QtAndroid"
#include "QAndroidJniEnvironment"
#endif
#include "QDebug"

/*
    Object Types
        Type            Signature
        jobject         Ljava/lang/Object;
        jclass          Ljava/lang/Class;
        jstring         Ljava/lang/String;      // 注意带有分号';'
        jthrowable      Ljava/lang/Throwable;
        jobjectArray	[Ljava/lang/Object;
        jarray          [<type>
        jbooleanArray	[Z
        jbyteArray      [B
        jcharArray      [C
        jshortArray     [S
        jintArray       [I
        jlongArray      [J
        jfloatArray     [F
        jdoubleArray	[D
    Primitive Types
        Type            Signature
        jboolean        Z
        jbyte           B
        jchar           C
        jshort          S
        jint            I
        jlong           J
        jfloat          F
        jdouble         D
    Other
        Type            Signature
        void            V
        Custom type     L<fully-qualified-name>;
 */

namespace Java {
    static int cutOutTop = 0;
    static int cutOutLeft = 0;
    static int cutOutBottom = 0;
    static int cutOutRight = 0;
    static QString hotspotSSID = QString("");
    static QString hotspotPassword = QString("");
}

#ifdef Q_OS_ANDROID
#ifdef __cplusplus
extern "C" {
#endif

//JNIEXPORT void JNICALL
//Java_org_qtproject_example_function_orientationChanged(JNIEnv */*env*/,
//                                                       jobject /*obj*/,
//                                                       int value)
//{
//    qDebug() << "orientationChanged:" << value;
//}

JNIEXPORT void JNICALL Java_org_qtproject_example_function_safeArea(JNIEnv */*env*/,
                                                                    jobject /*obj*/,
                                                                    int top, int left, int bottom, int right)
{
    Java::cutOutTop = top;
    Java::cutOutLeft = left;
    Java::cutOutBottom = bottom;
    Java::cutOutRight = right;
}

JNIEXPORT void JNICALL Java_org_qtproject_example_function_AndroidMessage(JNIEnv /*env*/,
                                                                    jobject /*obj*/,
                                                                    int msg, int wParam, int lParam)
{
    if( !ISVALID(g_Android) ) {
        return;
    }
    switch (msg) {
    case 0: {

    }
        break;
    default: break;
    }
}

#ifdef __cplusplus
}
#endif
#endif

class AndroidInterfacePrivate
{
public:
    explicit AndroidInterfacePrivate(AndroidInterface *parent = nullptr);
    ~AndroidInterfacePrivate();

#ifdef Q_OS_ANDROID
    QAndroidJniObject mainActivity;
    QAndroidJniEnvironment env;
#endif

    bool requestPermission(const QStringList &list);
    bool requestPermissionActivity(const QString &url);

private:
    AndroidInterface *f;
};

AndroidInterface::AndroidInterface(QObject *parent)
    : QObject(parent)
    , p(new AndroidInterfacePrivate(this))
{
    android_self = this;
}

AndroidInterface::~AndroidInterface()
{

}

void AndroidInterface::updateSetting()
{
    qDebug() << "hotspot status:" << hotspotOpen();
    emit updateHotspotInfo();
}

void AndroidInterface::setRotationScreen(const int &index)
{
    if( index < __unspecified
            || index > __portrait ) {
        return;
    }
#ifdef Q_OS_ANDROID
    jint orient = p->mainActivity.callMethod<jint>( "getRequestedOrientation" );   // 调用Android SDK方法，获取当前屏幕显示方向
    if(p->env->ExceptionCheck())       //异常捕获
    {
        qDebug() << "exception occured when get";
        p->env->ExceptionClear();
    }
    qDebug() << "set ratation:" << index << "current:" << orient;
    if( orient == index ) {
        return;
    }

    orient = index;
    p->mainActivity.callMethod<void>("setRequestedOrientation", "(I)V", orient);   // 调用Android SDK方法，设置屏幕方向
    if(p->env->ExceptionCheck())       //异常捕获
    {
        qDebug() << "exception occured when set";
        p->env->ExceptionClear();
    }
    qDebug() << "now screen orientation = " << orient;
#endif
}

void AndroidInterface::requestPhotoWritePermission()
{
#ifdef Q_OS_ANDROID
    bool flag = p->requestPermission(QStringList() << QString("android.permission.WRITE_EXTERNAL_STORAGE"));
    if( !flag ) {
        emit requestPermissionFail();
    }
#endif
}

int AndroidInterface::safeAeraLeft()
{
    return Java::cutOutLeft;
}

int AndroidInterface::safeAeraRight()
{
    return Java::cutOutRight;
}

int AndroidInterface::safeAreaTop()
{
    return Java::cutOutTop;
}

int AndroidInterface::safeAeraBottom()
{
    return Java::cutOutBottom;
}

bool AndroidInterface::hotspotOpen()
{
#ifdef Q_OS_ANDROID
    return p->mainActivity.callMethod<jboolean>("isApOpen");
//    return false;
#else
    return false;
#endif
}

QString AndroidInterface::hotspotSSID()
{
#ifdef Q_OS_ANDROID
//    QAndroidJniObject obj = p->mainActivity.callObjectMethod<jstring>("apSSID");
//    return obj.toString();
    return QString("");
#else
    return "";
#endif
}

QString AndroidInterface::hotspotPassword()
{
#ifdef Q_OS_ANDROID
//    QAndroidJniObject obj = p->mainActivity.callObjectMethod<jstring>("apPassword");
//    return obj.toString();
    return QString("");
#else
    return "";
#endif
}

void AndroidInterface::openHotspot()
{
#ifdef Q_OS_ANDROID
    p->mainActivity.callMethod<void>("turnToHotspot");
//    p->mainActivity.callMethod<void>("turnOnHotspot");
#else
    /* 热点开关测试 */
    if( Java::hotspotSSID.isEmpty()
            && Java::hotspotPassword.isEmpty() ) {
        Java::hotspotSSID = "hotspot ssid";
        Java::hotspotPassword = "hotspot password";
    }
    else {
        Java::hotspotSSID.clear();
        Java::hotspotPassword.clear();
    }
    emit updateHotspotInfo();
#endif
}

AndroidInterfacePrivate::AndroidInterfacePrivate(AndroidInterface *parent)
{
    f = parent;

#ifdef Q_OS_ANDROID
//    qDebug() << "android sdk version:" << QtAndroid::androidSdkVersion();
    mainActivity = QtAndroid::androidActivity();

    /*
    QAndroidJniObject className =
                mainActivity.callObjectMethod<jstring>("getLocalClassName");
    qDebug() << "java class name:" << className.toString();

    jint testInt = mainActivity.callMethod<jint>("testGetInt");
    qDebug() << "java test get int:" << testInt;
    */
#endif
}

AndroidInterfacePrivate::~AndroidInterfacePrivate()
{

}

bool AndroidInterfacePrivate::requestPermission(const QStringList &list)
{
#ifdef Q_OS_ANDROID
    for(QString permission : list) {
        QtAndroid::PermissionResult result = QtAndroid::checkPermission(permission);
        if( result == QtAndroid::PermissionResult::Denied )
        {
            QtAndroid::PermissionResultMap resultHash =
                    QtAndroid::requestPermissionsSync(QStringList() << permission);
            if( resultHash[permission] == QtAndroid::PermissionResult::Denied ) {
                qDebug() << QString("request %1 permission fail").arg(permission);
                return false;
            }
        }
        else {
            qDebug() << QString("%1 perssion is granted").arg(permission);
        }
    }
    return true;
#else
    return false;
#endif
}

bool AndroidInterfacePrivate::requestPermissionActivity(const QString &url)
{
    return true;
}

AndroidInterface * AndroidInterface::android_self = nullptr;
