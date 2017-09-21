/*
 * =====================================================================================
 *
 *       Filename:  extra_testing.cpp
 *
 *    Description: 
 *
 *        Version:  1.0
 *        Created:  05/29/2017 10:12:48 PM
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

std::shared_ptr<CSVHelper> helper;

int main(int , char*argv[]) {
    std::set<size_t> indexes = {3, 4, 5, 6, 17, 18, 19, 20, 21, 22, 23, 26, 28, 29, 31, 33, 61, 62, 64, 65, 66, 67, 72, 73};

    std::ofstream ofs0("extra_1.csv", std::ios_base::trunc);
    std::ofstream ofs1("extra_2.csv", std::ios_base::trunc);
    std::ofstream ofs2("extra_3.csv", std::ios_base::trunc);
    std::ofstream ofs3("extra_4.csv", std::ios_base::trunc);

    helper = std::make_shared<CSVHelper>(argv[1]);
    helper->split(4);

    std::thread thr0([&]() { helper->extract(0, indexes, ofs0); });
    std::thread thr1([&]() { helper->extract(1, indexes, ofs1); });
    std::thread thr2([&]() { helper->extract(2, indexes, ofs2); });
    std::thread thr3([&]() { helper->extract(3, indexes, ofs3); });

    thr0.join();
    thr1.join();
    thr2.join();
    thr3.join();

    Loger::log("共处理" + std::to_string(helper->record_count()) + "条记录");
    return 0;
}
