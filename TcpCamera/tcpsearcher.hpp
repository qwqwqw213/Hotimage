#ifndef TCPSEARCHER_H
#define TCPSEARCHER_H

#define TCP_SEARCH_THREAD_SIZE          5

#include "QTcpSocket"
#include "QThread"
#include "QTimer"
#include "QDebug"

class TcpSearcher : public QObject
{
    Q_OBJECT
public:
    TcpSearcher(const QString &_ip, const int &_port) : QObject(nullptr) {
        qDebug() << this->thread()->currentThreadId() << "search:" << _ip << _port;
        found = false;
        ip = _ip;
        port = _port;
        s = nullptr;
        for(int i = 0; i < TCP_SEARCH_THREAD_SIZE; i ++) {
            t[i] = new TcpSearcher::SearchThread(1 + i * 51, (i + 1) * 51,
                                                 this);
        }
    }
    TcpSearcher() : QObject(nullptr) {
        for(int i = 0; i < TCP_SEARCH_THREAD_SIZE; i ++) {
            t[i] = nullptr;
        }
        s = nullptr;
    }
    ~TcpSearcher() { }

    void search(const QString &_ip, const int &_port) {
        qDebug() << "search:" << _ip << _port;
        found = false;
        ip = _ip;
        port = _port;
        for(int i = 0; i < TCP_SEARCH_THREAD_SIZE; i ++) {
            if( t[i] == nullptr ) {
                t[i] = new TcpSearcher::SearchThread(1 + i * 51, (i + 1) * 51,
                                                     this);
            }
        }
    }

    QTcpSocket *socket() { return s; }

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
        SearchThread(const int &_min, const int &_max,
                     TcpSearcher *_p) : QThread(nullptr) {
            min = _min;
            max = _max;
            p = _p;
            start();
        }
        ~SearchThread() { }

    protected:
        void run() override {
            QTcpSocket *s = new QTcpSocket;
            QString search = p->ip.left(p->ip.lastIndexOf('.') + 1);
            int index = min;
            while (!this->isInterruptionRequested() && !p->found) {
                QString devIp = search + QString::number(index);
                if( devIp == p->ip ) {
                    index ++;
                    continue;
                }

                s->connectToHost(devIp, p->port);
                s->waitForConnected(50);
                if( s->state() == QTcpSocket::ConnectedState ) {
                    p->found = true;
                    s->moveToThread(p->thread());
                    p->s = s;
                    emit p->finished(devIp, p->ip);
                    break;
                }

                index ++;
                if( index > max ) {
                    index = min;
                }
            }

            if( s->state() != QTcpSocket::ConnectedState ) {
                s->deleteLater();
                s = nullptr;
            }
        }

    private:
        int min;
        int max;
        TcpSearcher *p;
    };

    SearchThread *t[TCP_SEARCH_THREAD_SIZE];
    bool found;
    QString ip;
    int port;
    QTcpSocket *s;

Q_SIGNALS:
    void finished(const QString &devIp, const QString &localIp);
};

#endif
