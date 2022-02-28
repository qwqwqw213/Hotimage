#include "wireless.h"

#include "QDebug"

class WirelessPrivate
{

public:
    explicit WirelessPrivate(Wireless *parent);
    ~WirelessPrivate();

    QStringList nameList;

    void scan();

private:
    Wireless *f;
};

Wireless::Wireless(QObject *parent)
    : QObject(parent)
    , p(new WirelessPrivate(this))
{

}

Wireless::~Wireless()
{

}

void Wireless::scan()
{
    p->scan();
}

WirelessPrivate::WirelessPrivate(Wireless *parent)
{
    f = parent;
}

WirelessPrivate::~WirelessPrivate()
{

}

void WirelessPrivate::scan()
{

}
