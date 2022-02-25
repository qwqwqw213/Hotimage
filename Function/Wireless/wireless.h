#ifndef WIRELESS_H
#define WIRELESS_H

#include <QObject>

class WirelessPrivate;
class Wireless : public QObject
{
    Q_OBJECT

public:
    explicit Wireless(QObject *parent = nullptr);
    ~Wireless();

    void scan();

private:
    friend class WirelessPrivate;
    QScopedPointer<WirelessPrivate> p;
};

#endif // WIRELESS_H
