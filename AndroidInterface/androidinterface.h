#ifndef ANDROIDINTERFACE_H
#define ANDROIDINTERFACE_H

#include <QObject>

class AndroidInterfacePrivate;
class AndroidInterface : public QObject
{
    Q_OBJECT

public:
    explicit AndroidInterface(QObject *parent = nullptr);
    ~AndroidInterface();

    enum RotationScreen
    {
        __unspecified = -1,      // 跟随系统
        __landscape,            // 横向
        __portrait,             // 纵向
    };
    Q_INVOKABLE void setRotationScreen(const int &index);

private:
    friend class AndroidInterfacePrivate;
    QScopedPointer<AndroidInterfacePrivate> p;
};

#endif // ANDROIDINTERFACE_H
