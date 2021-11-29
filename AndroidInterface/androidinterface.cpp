#include "androidinterface.h"

#ifdef Q_OS_ANDROID
#include "QAndroidJniObject"
#include "QtAndroid"
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
        jboolean            Z
        jbyte               B
        jchar               C
        jshort              S
        jint                I
        jlong               J
        jfloat              F
        jdouble             D
    Other
        Type        Signature
        void        V
        Custom type	L<fully-qualified-name>;
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
}

AndroidInterface::~AndroidInterface()
{

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
