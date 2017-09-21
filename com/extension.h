/*
 * =====================================================================================
 *
 *       Filename:  cpp_extension.h
 *
 *    Description:  对C++的扩充
 *
 *        Version:  1.0
 *        Created:  05/21/2017 12:24:54 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __CPP_EXTENSION_INCLUDE
#define __CPP_EXTENSION_INCLUDE
#include <string>
#include <vector>
#include <sstream>
#include <netdb.h>

template<typename T>
static inline std::string to_string(const std::vector<T>& c) {
    std::string res = "{";
    std::ostringstream oss;
    oss << "{";
    for (auto it = c.begin(); it != c.end(); ++it) {
	oss << "0x" << std::hex << (int)*it;
	if (it != c.end() - 1)
	    oss << ", ";
    }
    oss << "}";
    return oss.str();
}

static inline uint64_t ntohll(uint64_t v) {
    uint64_t upper = v & 0xFFFFFFFF00000000; 
    uint64_t lower = v & 0x00000000FFFFFFFF;
    upper >>= 32;
    upper = ::ntohl((uint32_t)upper);
    lower = ::ntohl((uint32_t)lower);
    v = (lower << 32) | upper;
    return v;
}

static inline  std::ostream& operator<<(std::ostream& os, std::vector<std::string>& ss) {
    for (size_t i = 0; i < ss.size(); ++i) {
	os << ss[i];
	if (i != ss.size() - 1)
	    os << ",";
    }
    os << std::endl;
    return os;
}
#endif // __CPP_EXTENSION_INCLUDE

