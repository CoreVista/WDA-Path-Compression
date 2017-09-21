/*
 * =====================================================================================
 *
 *       Filename:  dir_testing.cpp
 *
 *    Description: =
 *
 *        Version:  1.0
 *        Created:  05/28/2017 10:04:36 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <algorithm>
#include <atomic>
#include <pthread.h>
#include "../com/loger.h"
#include "../terminal/file.h"

std::shared_ptr<CSVHelper> helper;
std::atomic_size_t count;

std::array<std::vector<std::vector<std::string>>, 4> records;

void * thread_helper(void *param) {
    size_t thr = reinterpret_cast<size_t>(param);
    std::vector<std::string> fields;
    size_t progress = 0;
    while (helper->next_record(thr, fields)) {
	//records[thr].push_back(std::move(fields));
	count ++;
	if (progress != helper->progress(thr)) {
	    progress = helper->progress(thr);
	    Loger::log("Thread: " + std::to_string(thr) + " Progress:" + std::to_string(progress) + "%");
	}
    }
    return nullptr;
}

int main(int , char *argv[]) {
    std::vector<std::string> fields;
    helper = std::make_shared<CSVHelper>(argv[1]);
    helper->split(4);

    std::vector<pthread_t> thrds;
    thrds.assign(4, {});
    for (size_t i = 0; i < 4; ++i) { pthread_create(&thrds[i], nullptr, thread_helper, (void*)i); }
    for (size_t i = 0; i < 4; ++i) { pthread_join(thrds[i], nullptr); }

    Loger::log("处理完毕, 一共" + std::to_string(count) + "条记录");
    Loger::wait();
    return 0;
}
