/*
 * =====================================================================================
 *
 *       Filename:  sort_test.cpp
 *
 *    Description: j
 *
 *        Version:  1.0
 *        Created:  05/30/2017 01:56:39 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <thread>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../terminal/file.h"

int main(int , char*argv[]) {
    mkdir(argv[2], 0776);
    errno = 0;
    std::string command = std::string("rm ") + argv[2] + "/*";
    system(command.c_str());

    errno = 0;
    std::shared_ptr<CSVHelper> helper;

    helper = std::make_shared<CSVHelper>(argv[1]);
    helper->split(helper->csvfile_count());

    for (size_t i = 0; i < helper->csvfile_count(); ++i) {
	if (i == 75) {
	    Loger::log("stop");
	}
	Loger::log("正在排序第" + std::to_string(i) + "文件:" + helper->filename(i));
	helper->sort(i, argv[2]);
    }

    Loger::log("执行完毕");
    Loger::wait();
    return 0;
}
