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

private:
    friend class AndroidInterfacePrivate;
    QScopedPointer<AndroidInterfacePrivate> p;
};

#endif // ANDROIDINTERFACE_H
