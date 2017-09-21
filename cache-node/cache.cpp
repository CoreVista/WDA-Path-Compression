/*
 * =====================================================================================
 *
 *       Filename:  cache.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/30/2017 05:45:00 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "cache.h"

using namespace std;

void Cache::push_back(const VehicleData& vd) {
    countOfProcessedPackage += 1;
    std::shared_ptr<Vehicle> target;
    {
	lmap1.rlock();
	auto fit = vehicles.find(vd.key1);
	lmap1.unlock();
	if (fit == vehicles.end()) {
	    target = std::make_shared<Vehicle>(vd.id);
	    auto vmap1 = std::make_pair(vd.key1, std::map<uint64_t, std::shared_ptr<Vehicle>>());
	    vmap1.second[vd.key2] = target;
	    lmap1.wlock();
	    vehicles.insert(vmap1);
	    lmap1.unlock();
	    //unfindCounter += 1;
	    //Loger::log("未找到" + std::to_string((size_t)unfindCounter));
	}
	else {
	    auto map2 = fit->second;
	    lmap2.rlock();
	    auto res = map2.find(vd.key2);
	    lmap2.unlock();
	    if (res == fit->second.end()) {
		target = std::make_shared<Vehicle>(vd.id);
		lmap2.wlock();
		map2[vd.key2] = target;
		lmap2.unlock();
	    }
	    else {
		target = res->second;
	    }
	}
    }

    if (target == nullptr) {
	Loger::err("Unreached");
	return;
    }
    target->push_back(vd);
   // Loger::log(target->to_string());
}

void Cache::statistic() {
    using namespace std;
    static vector<size_t> counter(21, 0);
    vector<size_t> counters(21, 0);
    vector<string> titles = {
	"位置",
	"车门1",
	"车门2",
	"左转向灯",
	"右转向灯",
	"光灯",
	"远光灯",
	"前雾灯",
	"刹车灯",
	"雨刷",
	"缓速器",
	"刹车信号",
	"风扇开关",
	"跛行状态",
	"开门滑行",
	"急加速",
	"急减速",
	"空档滑行",
	"油门过大",
	"空调制冷",
	"通风"
    };

    counters = countOfCompressed;
    Loger::log			     ("|                                         压缩率实验结果                                              |");
    char buffer[512];
    snprintf (buffer, sizeof(buffer), "-------------------------------------------------------------------------------------------------------");
    Loger::log(buffer);
    snprintf (buffer, sizeof(buffer),"|%-20s\t|%-20s\t|%-20s\t|\t%20s|", "字段", "压缩前条数", "压缩后条数", "压缩比");
    Loger::log(buffer);
    snprintf (buffer, sizeof(buffer), "-------------------------------------------------------------------------------------------------------");
    Loger::log(buffer);
    for (size_t i = 0; i < counters.size(); ++i) {
	snprintf (buffer, sizeof(buffer), "|%-20s\t|%-20lu\t|%-20lu\t|\t%20lu%%|", 
		titles[i].c_str(), (size_t)countOfProcessedPackage, counters[i], ((countOfProcessedPackage - counters[i]) * 100 / countOfProcessedPackage));
	Loger::log(buffer);
    }
    snprintf(buffer, sizeof(buffer), "-------------------------------------------------------------------------------------------------------");
    Loger::log(buffer);
}

void Cache::print() {
    for (auto& m: vehicles) {
	for (auto& v: m.second) {
	    std::cout << v.second->to_string() << std::endl;
	}
    }
}

void Cache::write_to_file(std::string outDir) {
    Loger::log("正在将已压缩数据写出到磁盘..." + outDir);
    for (auto& m1 : vehicles) {
	for (auto &m2 : m1.second) {
	    m2.second->write_to_file(outDir);
	}
    }
    Loger::log("写出完毕，压缩数据已经保存在" + outDir + "文件夹");
}

std::shared_ptr<Vehicle> Cache::get_vehicle(const std::string& id ) {
    try {
	auto keys = generate_index_from_vehicle_id(id);
	AutoRlocker rlock0(lmap1);
	auto& m1 = vehicles.at(keys.first);
	{
	    AutoRlocker rlock1(lmap2);
	    auto& m2 = m1.at(keys.second);
	    return m2;
	}
    }
    catch (std::out_of_range& oe) {
	return nullptr;
    }
}


bool Cache::count() {
    vector<size_t> counts(21, 0);
    lmap1.rlock();
    lmap2.rlock();
    for (auto& m1 : vehicles) {
	for (auto &m2 : m1.second) {
	    shared_ptr<Vehicle> v = m2.second;
	    counts[0] += v->locationTimeline.size();
	    for (size_t i = 0; i < v->generalTimelines.size(); ++i) {
		counts[i + 1] += v->generalTimelines[i].size();
	    }
	}
    }
    lmap2.unlock();
    lmap1.unlock();
    for (size_t i = 0; i < counts.size(); ++i) {
	if (counts[i] > countOfCompressed[i]) {
	    countOfCompressed = std::move(counts);
	    return true;
	}
    }
    return false;
}
