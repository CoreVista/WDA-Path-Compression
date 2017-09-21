// © 2016 lenfien

//
// Created by lenfien on 16-10-21.
//

#ifndef BUG_QUEUE_H
#define BUG_QUEUE_H

#include <pthread.h>
#include <deque>
#include "../com/locker.h"

template <typename T>
class Queue {
private:
    Condlock _condLock;
    std::deque<T> _queue;

public:
    /* May block */
    void get(T& t) {
        Autolock<Condlock> autolock(_condLock);
        while (_queue.size() == 0)
            _condLock.wait();
        t = _queue.front();
        _queue.pop_front();
    }

    void put(const T &v) {
        Autolock<Condlock> autolock (_condLock);
        _queue.push_back(v);
        _condLock.signal();
    }

    bool try_pop(T& t) {
        Autolock<Condlock> autoLock(_condLock);
        if (_queue.size() == 0)
            return false;
        t = _queue.front();
        _queue.pop_front();
        return true;
    }

    size_t count() {
        Autolock<Condlock> autol (_condLock);
        return _queue.size();
    }
};

#endif //BUG_QUEUE_H
