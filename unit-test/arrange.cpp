/*
 * =====================================================================================
 *
 *       Filename:  arrange_test.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/30/2017 11:34:02 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <thread>
#include "../terminal/file.h"


int main(int , char*argv[]) {
    std::set<size_t> indexes = {3, 4, 5, 6, 17, 18, 19, 20, 21, 22, 23, 26, 28, 29, 31, 33, 61, 62, 64, 65, 66, 67, 72, 73};

    std::shared_ptr<CSVHelper> helper;
    helper = std::make_shared<CSVHelper>(argv[1]);
    helper->split(1);
    helper->arrange(argv[2]);

    Loger::log("共处理" + std::to_string(helper->record_count()) + "条记录");
    Loger::log("执行完毕");

    Loger::wait();
    return 0;
}
