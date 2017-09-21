// © 2016 lenfien

//
// Created by lenfien on 16-10-21.
//

#ifndef BUG_LOG_H
#define BUG_LOG_H

#include <string>
#include <thread>
#include <iostream>
#include <fstream>
#include "queue.h"
#include "../com/error.h"

class Loger {
    enum LogType {
        LOG, WARNING, ERROR, FATAL
    };

    static bool _logStarted;
    static std::ostream& _logofs;

private:
    static Queue<std::pair<std::string, LogType>> _msgQue;
    static pthread_t _logThread;

private:
    static void apend(const std::string &msg, LogType type) {
        static __thread char timebuf[512];
        static __thread char num[12];
        static __thread struct timeval tv;
        static __thread struct tm tm_;

        if (!_logStarted) { start(); }

        std::string str;
        str.clear();
        gettimeofday (&tv, nullptr);
        localtime_r (&tv.tv_sec, &tm_);
        strftime(timebuf, sizeof(timebuf), "%F %T", &tm_);
        snprintf (num, sizeof (num), "%-6lu", tv.tv_usec);

        str += "[";
        str += timebuf;
        str += ".";
        str += num;
        str += "] : ";
        str += msg;

        _msgQue.put(std::make_pair(str, type));
    }

public:
    static void log (std::string msg) {
        if (errno != 0) {
            msg += "<";
            msg += strerror(errno);
            msg += ">";
        }
        apend(msg, LogType::LOG);
    }

    static void err (std::string msg) {
        if (errno != 0) {
            msg += "<";
            msg += strerror(errno);
            msg += ">";
        }
        apend(msg, LogType::ERROR);
    }

    static void warning (std::string msg) {
        if (errno != 0) {
            msg += "<";
            msg += strerror(errno);
            msg += ">";
        }
        apend(msg, LogType::WARNING);
    }

    static void fatal (std::string msg) {
        if (errno != 0) {
            msg += "<";
            msg += strerror(errno);
            msg += ">";
        }
        apend(msg, LogType::FATAL);
    }

    static void start() {
        if (!_logStarted)
            _logStarted = true;
        pthread_create (&_logThread, nullptr, log_thread, nullptr);
    }

    static void stop () {
        std::cout << "Log System Stopped." << std::endl;
        pthread_cancel(_logThread);
        pthread_join(_logThread, nullptr);
    }

    static void wait() {
	pthread_join(_logThread, nullptr);
    }

private:
    static void * log_thread (void*){
        std::pair<std::string, LogType> log;
	
        while (true) {
            _msgQue.get(log);
            switch (log.second) {
                case LOG:
//                    _logofs << "[LOG]" << log.first << std::endl;
                    std::cout << "[LOG]" << log.first << std::endl;
                    break;
                case ERROR:
//                   _logofs << "[ERR]" << log.first << std::endl;
                    std::cerr << "[ERR]" << log.first << std::endl;
                    break;
                case FATAL:
//                    _logofs << "[FAT]" << log.first << std::endl;
                    std::cerr << "[FAT]" << log.first << std::endl;
                    break;
                case WARNING:
//                    _logofs << "[WAR]" << log.first << std::endl;
                    std::cerr << "[WAR]" << log.first << std::endl;
                    break;
                default:
                    break;
            }
        }
    }
};


#endif //BUG_LOG_H
