//
// Created by lenfien on 16-10-25.
//

#ifndef BUG_EXP_H
#define BUG_EXP_H

#include <typeinfo>
#include <string>
#include <exception>
#include <cstring>

#define CODE_POSITION (std::string("FUNC(") + std::string(__FUNCTION__) + ")->POS(" + std::to_string(__LINE__)+")")

class Error : public std::exception {
public:
    std::string _code_position;
    mutable std::string _reason;
    mutable std::string _what;

    int _errno;

    Error () {
        _errno = errno;
    }

    virtual const char *what () const throw() {
        if (_errno != 0) {
            _reason += "<";
            _reason += strerror(_errno);
            _reason += ">";
        }
        _what = (std::string("[") + std::string(typeid(*this).name()) + "] throwed from [" + _code_position + "] because of [" + _reason + "]");
        return _what.c_str();
    }
};

#define throw_exp(Exp, why) { \
    Exp _err; \
    _err._code_position = (CODE_POSITION); \
    _err._reason = (why); \
    throw _err;  \
}

#endif //BUG_EXP_H
