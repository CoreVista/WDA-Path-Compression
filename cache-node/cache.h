/*
 * =====================================================================================
 *
 *       Filename:  cache.h
 *
 *    Description: 
 *
 *        Version:  1.0
 *        Created:  05/24/2017 03:19:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __INCLUDE_CACHE
#define __INCLUDE_CACHE

#include <time.h>
#include <vector>
#include <map>
#include <memory>
#include <atomic>
#include <unistd.h>

#include "../com/locker.h"
#include "data.h"
#include "compress.h"

class Cache {
private:
    Rwlocker lmap1, lmap2;
    std::map<uint64_t, std::map<uint64_t, std::shared_ptr<Vehicle>>> vehicles;
    std::atomic_size_t countOfProcessedPackage;	 /* 处理过VehicleData计数 */
    std::vector<size_t> countOfCompressed;
    std::shared_ptr<std::thread> reportThread;

public:
    Cache() {
	countOfProcessedPackage = 0;
	countOfCompressed.resize(21, 0);
//	reportThread = std::make_shared<std::thread>([this]() {
//	    static size_t prevCountOfProcessedPackaged = 0;
//	    while (true) {
//		if(count()) {
//		    this->statistic();
//		    size_t speed = (countOfProcessedPackage - prevCountOfProcessedPackaged) * 1000 / 300;
//		    prevCountOfProcessedPackaged = countOfProcessedPackage;
//		    Loger::log ("本次统计吞吐量:" + std::to_string(speed) + "条/秒");
//		}
//		usleep(1000000);
//	    }
//	});
    }

    /* 将一个vd加入到缓存中 */
    void push_back(const VehicleData& vd);

    /* 输出所有vd */
    void print();

    /* 输出统计信息 */
    void statistic();

    /* 计数 */
    bool count();

    /* 将所有车辆数据写入的目录outDir下，一辆车一个目录 */
    void write_to_file(std::string outDir);

    /* 获取一个车辆的信息 */
    std::shared_ptr<Vehicle> get_vehicle(const std::string& id);
};

#endif 
