/*
 * =====================================================================================
 *
 *       Filename:  Worker.h
 *
 *    Description:  线程池
 *			1. 执行缓存系统的压缩任务与生成索引工作
 *			2. 向令牌系统申请续租，并响应续租结果
 *
 *        Version:  1.0
 *        Created:  05/22/2017 02:07:26 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __INCLUDE_WORKER
#define __INCLUDE_WORKER

#include <vector>
#include <pthread.h>
#include "data.h"

#include "../com/loger.h"
#include "../com/queue.h"
#include "cache.h"

class WorkerPool {
private:
    std::vector<::pthread_t>	workerThreads;
    std::shared_ptr<Cache>	pCache;
    Queue<std::shared_ptr<Message>>& inQueue;
    Queue<std::shared_ptr<Message>>& outQueue;

public:
    WorkerPool(std::shared_ptr<Cache> cache, Queue<std::shared_ptr<Message>>& in, Queue<std::shared_ptr<Message>>& out)
	:pCache(cache), inQueue(in), outQueue(out) {
	for (int i = 0; i < 1; ++i) {
	    pthread_t thr;
	    pthread_create(&thr, nullptr, worker_thread, this);
	    workerThreads.push_back(thr);
	}
    }

private:
    static void* worker_thread(void *p) {
	using namespace std;
	WorkerPool *pself = reinterpret_cast<WorkerPool*>(p);
	Cache *cache = pself->pCache.get();
	auto& inQueue = pself->inQueue;
	//auto& outQueue = pself->outQueue;
	Loger::log("work online");
	for (;;) {
	    std::shared_ptr<Message> msg;
	    inQueue.get(msg);
	    cache->push_back(*msg);
	}
    }
};

#endif
