#ifndef TCPSEARCHER_H
#define TCPSEARCHER_H

#define TCP_SEARCH_THREAD_SIZE          5

#include "QTcpSocket"
#include "QThread"
#include "QDebug"

class TcpSearcher : public QObject
{
    Q_OBJECT
public:
    TcpSearcher(const QString &ip, const int &port) : QObject(nullptr) {
        found = false;
        for(int i = 0; i < TCP_SEARCH_THREAD_SIZE; i ++) {
            t[i] = new SearchThread(ip, port,
                                    1 + i * 51, (i + 1) * 51,
                                    this);
        }
        QObject::connect(this, &TcpSearcher::finished,
                         this, [=](){
            release();
        }, Qt::QueuedConnection);
    }
    TcpSearcher() {
        for(int i = 0; i < TCP_SEARCH_THREAD_SIZE; i ++) {
            t[i] = nullptr;
        }
        QObject::connect(this, &TcpSearcher::finished,
                         this, [=](){
            release();
        }, Qt::QueuedConnection);
    }
    ~TcpSearcher() { }

    void search(const QString &ip, const int &port) {
        found = false;
        for(int i = 0; i < TCP_SEARCH_THREAD_SIZE; i ++) {
            if( t[i] == nullptr ) {
                t[i] = new SearchThread(ip, port,
                                        1 + i * 51, (i + 1) * 51,
                                        this);
            }
        }
    }

    void release() {
        for(int i = 0; i < TCP_SEARCH_THREAD_SIZE; i ++) {
            if( t[i] != nullptr ) {
                if( t[i]->isRunning() ) {
                    t[i]->requestInterruption();
                    t[i]->wait();
                    t[i]->deleteLater();
                    t[i] = nullptr;
                }
            }
        }
    }

private:
    class SearchThread : public QThread
    {
    public:
        SearchThread(const QString &_ip, const int &_port,
                     const int &_min, const int &_max,
                     TcpSearcher *_p) : QThread(nullptr) {
            ip = _ip;
            min = _min;
            max = _max;
            port = _port;
            p = _p;
            qDebug() << "search range:" << min << max;
            start();
        }
        ~SearchThread();

    protected:
        void run() override {
            QTcpSocket *s = new QTcpSocket;
            QString search = ip.left(ip.lastIndexOf('.') + 1);
            int index = min;
            while (!this->isInterruptionRequested() && !p->found) {
                QString devIp = search + QString::number(index);
                if( devIp == ip ) {
                    index ++;
                    continue;
                }

                s->connectToHost(devIp, port);
                s->waitForConnected(50);
                if( s->state() == QTcpSocket::ConnectedState ) {
                    p->found = true;
                    emit p->finished(devIp);
                    break;
                }

                index ++;
                if( index > max ) {
                    index = min;
                }
            }

            s->deleteLater();
            s = nullptr;
        }

    private:
        int min;
        int max;
        QString ip;
        int port;
        TcpSearcher *p;
    };

    SearchThread *t[TCP_SEARCH_THREAD_SIZE];
    bool found;

Q_SIGNALS:
    void finished(const QString &devIp);
};

#endif
