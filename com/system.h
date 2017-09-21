//
// Created by lenfien on 16-11-6.
//

#ifndef BUG_SYSTEM_H
#define BUG_SYSTEM_H

#include <string>
#include <sys/time.h>

class System {
public:
    static void now(std::string& _result) {
        static __thread char buffer[128];
        time_t t = time(nullptr);
        struct tm _tm;
        localtime_r(&t, &_tm);
        strftime (buffer, sizeof (buffer), "%F %T", &_tm);
        _result = buffer;
    }

    static void uid(std::string& _uid) {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        _uid = std::to_string (tv.tv_sec);
        _uid += std::to_string (tv.tv_usec);
        _uid += std::to_string (rand());
    }

    static inline bool is_big_endian() {
	union U { int32_t i = 1; char cs[4]; };
	return U().cs[0] == 0;
    }

    static time_t start_time() {
	static time_t startTime = time(nullptr);
	return startTime;
    }
};


#endif //BUG_SYSTEM_H
