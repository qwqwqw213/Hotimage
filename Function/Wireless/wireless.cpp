#include "wireless.h"

#include "QNetworkConfigurationManager"
#include "QDebug"

class WirelessPrivate
{

public:
    explicit WirelessPrivate(Wireless *parent);
    ~WirelessPrivate();

    QList<QNetworkConfiguration> configList;
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
    QNetworkConfigurationManager ncm;
    configList = ncm.allConfigurations();
    nameList.clear();

    for( auto &cfg : configList )
    {
        if( cfg.bearerType() == QNetworkConfiguration::BearerWLAN ) {
            if( !cfg.name().isEmpty() ) {
                nameList.append(cfg.name());
            }
            qDebug() << "wifi type:" << cfg.type() << cfg.name();
        }
    }
}
