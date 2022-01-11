#include "androidinterface.h"

#ifdef Q_OS_ANDROID
#include "QAndroidJniObject"
#include "QtAndroid"
#include "QAndroidJniEnvironment"
#include "QDebug"
#endif

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

#ifdef Q_OS_ANDROID
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_org_qtproject_example_function_orientationChanged(JNIEnv */*env*/,
                                                       jobject /*obj*/,
                                                       int value)
{
    qDebug() << "orientationChanged:" << value;
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
    QAndroidJniObject obj;
#endif

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

void AndroidInterface::setRotationScreen(const int &index)
{
    if( index < __unspecified
            || index > __portrait ) {
        return;
    }
#ifdef Q_OS_ANDROID
    QAndroidJniEnvironment env;
    QAndroidJniObject activity = QtAndroid::androidActivity();

    jint orient = activity.callMethod<jint>( "getRequestedOrientation" );   // 调用Android SDK方法，获取当前屏幕显示方向
    if(env->ExceptionCheck())       //异常捕获
    {
        qDebug() << "exception occured when get";
        env->ExceptionClear();
    }
    qDebug() << "set ratation:" << index << "current:" << orient;
    if( orient == index ) {
        return;
    }

    orient = index;
    activity.callMethod<void>("setRequestedOrientation", "(I)V", orient);   // 调用Android SDK方法，设置屏幕方向
    if(env->ExceptionCheck())       //异常捕获
    {
        qDebug() << "exception occured when set";
        env->ExceptionClear();
    }
    qDebug() << "now screen orientation = " << orient;
#endif
}

void AndroidInterface::requestPhotoWritePermission()
{
#ifdef Q_OS_ANDROID
    QtAndroid::PermissionResult result = QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE");
    if( result == QtAndroid::PermissionResult::Denied ) {
        QtAndroid::PermissionResultMap resultHash = QtAndroid::requestPermissionsSync(QStringList({"android.permission.WRITE_EXTERNAL_STORAGE"}));
        if( resultHash["android.permission.WRITE_EXTERNAL_STORAGE"] == QtAndroid::PermissionResult::Denied ) {
            qDebug() << "request write permission success";
        }
    }
    else {
//        qDebug() << "has write permission";
    }
#endif
}

AndroidInterfacePrivate::AndroidInterfacePrivate(AndroidInterface *parent)
{
    f = parent;

#ifdef Q_OS_ANDROID

#endif
}

AndroidInterfacePrivate::~AndroidInterfacePrivate()
{

}

AndroidInterface * AndroidInterface::android_self = nullptr;
