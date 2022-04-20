#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "QMutex"
#include "QQueue"

template <typename T>
class Queue
{
public:
    Queue() { q.clear(); }
    ~Queue() { q.clear(); }

    void enqueue(const T &d) {
        m.lock();
        q.enqueue(d);
        m.unlock();
    }
    T dequeue() {
        m.lock();
        T d = q.dequeue();
        m.unlock();
        return d;
    }
    int size() { return q.size(); }
    void clear() {
        m.lock();
        q.clear();
        m.unlock();
    }

private:
    QQueue<T> q;
    QMutex m;
};

#endif
