// © 2016 lenfien

//
// Created by lenfien on 16-10-21.
//

#ifndef BUG_LOCKER_H
#define BUG_LOCKER_H

#include <pthread.h>
#include <stdexcept>
#include <sys/time.h>
#include "error.h"

class LockerError : public Error {};

class Mutex {
private:
    pthread_mutex_t mtx;

public:
    Mutex () throw (LockerError) { if (pthread_mutex_init (&mtx, nullptr) != 0) throw LockerError(); }
    ~Mutex() { pthread_mutex_destroy(&mtx); }

    inline void lock() { pthread_mutex_lock (&mtx); }
    inline void unlock() { pthread_mutex_unlock(&mtx); }
};

class Rwlocker {
private:
    pthread_rwlock_t rwl;

public:
    Rwlocker() throw (LockerError) { if (pthread_rwlock_init (&rwl, nullptr) != 0) throw LockerError(); }
    ~Rwlocker() { pthread_rwlock_destroy(&rwl); }

    inline void rlock() { pthread_rwlock_rdlock(&rwl); }
    inline void wlock() { pthread_rwlock_wrlock(&rwl); }
    inline void unlock() { pthread_rwlock_unlock(&rwl); }
};


class Condlock {
private:
    pthread_mutex_t mtx;
    pthread_cond_t cond;

public:
    Condlock() {
        if (pthread_mutex_init (&mtx, nullptr) != 0) throw LockerError();
        if (pthread_cond_init (&cond, nullptr) != 0) throw LockerError();
    }

    ~Condlock() {
        pthread_mutex_destroy(&mtx);
        pthread_cond_destroy(&cond);
    }

    inline void signal () { pthread_cond_signal(&cond); }
    inline void broadcast () { pthread_cond_broadcast(&cond); }
    inline void lock () { pthread_mutex_lock (&mtx); }
    inline void unlock () { pthread_mutex_unlock(&mtx); }
    inline void wait() { pthread_cond_wait (&cond, &mtx); }

    inline int timed_wait(unsigned long ms) {
        static __thread struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ts.tv_sec += ms / 1000;
        ts.tv_nsec += (ms % 1000) * 1000000;
        return pthread_cond_timedwait(&cond, &mtx, &ts);
    }
};

template <typename T>
class Autolock {
private:
    T& _t;

public:
    Autolock(T& t):_t (t) { _t.lock();}
    ~Autolock() { _t.unlock(); }
};

class AutoRlocker {
private:
    Rwlocker& _rwl;

public:
    AutoRlocker(Rwlocker& rwl):_rwl(rwl) {_rwl.rlock();}
    ~AutoRlocker() { _rwl.unlock(); }
};

class AutoWlocker {
private:
    Rwlocker& _rwl;

public:
    AutoWlocker(Rwlocker& rwl):_rwl(rwl) {_rwl.wlock();}
    ~AutoWlocker() { _rwl.unlock(); }
};

#endif //BUG_LOCKER_H
