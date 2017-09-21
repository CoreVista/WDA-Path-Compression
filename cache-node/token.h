/*
 * =====================================================================================
 *
 *       Filename:  token.h
 *
 *    Description: 令牌处理
 *		    1. 令牌有效检查
 *		    2. 异步令牌续租
 *		    3. 令牌续租结果通知
 *
 *
 *        Version:  1.0
 *        Created:  05/21/2017 12:13:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __INCLUDE_TOKEN
#define __INCLUDE_TOKEN

#include <map>
#include <ctime>
#include <mutex>
#include <pthread.h>
#include <functional>

#include "../com/locker.h"
#include "../com/types.h"
#include "data.h"

class TokenManager {
    using RequestRenewCallback = std::function<void(uint64_t, Node*)>;

private:
    Rwlocker rwLocker_tokens;
    std::map<uint64_t, std::time_t> tokens;
    Queue<uint64_t> reqRenewTokens;         /* 请求续租的令牌队列 */
    Rwlocker rwlocker_callBacks;
    std::map<uint64_t, std::shared_ptr<RequestRenewCallback>> callbacks;

public:
    bool is_token_valid(uint64_t token) {
	AutoRlocker lock (rwLocker_tokens);
	time_t now;
	time(&now);
	auto res = tokens.find(token);
	if (res != tokens.end())
	    return now < res->second;
	return false;
    }

    void request_valid_token(uint64_t token, std::shared_ptr<RequestRenewCallback> callback) {
	AutoWlocker locker (rwlocker_callBacks);
	callbacks[token] = callback;
    }
};

#endif
