/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/30/2017 05:26:33 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <chrono>
#include <mysql_driver.h>
#include <cppconn/driver.h>
#include <mysql_connection.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <arpa/inet.h>
#include "../cache-node/cache.h"
#include "../cache-node/data.h"
#include "../com/extension.h"
#include "file.h"

using namespace std;

void show_usage() { printf("terminal <root dir> <node-ip> <service|port>\n"); }

void cache_test(Cache& cache, CSVHelper& helper) {
    helper.split(helper.csvfile_count());

    Loger::log("----------------------------------------压缩率测试开始-------------------------------------------------");
    Loger::log("-------------------------------------------------------------------------------------------------------");
    auto sbeg = chrono::high_resolution_clock::now();
    atomic_size_t count;
    count = 0;
    thread thr0 ([&count, &helper, &cache]() {
	vector<string> fields;
	for (size_t i = 0; i < 30 && i < helper.csvfile_count(); ++i) {
	    auto cbeg = chrono::high_resolution_clock::now();
	    while (helper.next_record(i, fields)) {
		VehicleData vd(fields);
		cache.push_back(vd);
	    }
	    count += 1;
	    auto cent = chrono::high_resolution_clock::now();
	    auto len = chrono::duration_cast<chrono::milliseconds>(cent - cbeg).count();
	    Loger::log("已缓存-压缩车辆(" + to_string(count) + "/" + to_string(helper.csvfile_count()) + ")" + "\t用时" + to_string((len)) + "ms"+ "  \t[" + helper.filename(i) + "])");
	}
    });

    thread thr1 ([&count, &helper, &cache]() {
	vector<string> fields;
	for (size_t i = 30; i < 60 && i < helper.csvfile_count(); ++i) {
	    auto cbeg = chrono::high_resolution_clock::now();
	    while (helper.next_record(i, fields)) {
		VehicleData vd(fields);
		cache.push_back(vd);
	    }
	    auto cent = chrono::high_resolution_clock::now();
	    auto len = chrono::duration_cast<chrono::milliseconds>(cent - cbeg).count();
	    count += 1;
	    Loger::log("已缓存-压缩车辆(" + to_string(count) + "/" + to_string(helper.csvfile_count()) + ")" + "\t用时" + to_string((len)) + "ms"+ "  \t[" + helper.filename(i) + "])");
	}
    });

    thread thr2 ([&count, &helper, &cache]() {
	vector<string> fields;
	for (size_t i = 60; i < 90 && i < helper.csvfile_count(); ++i) {
	    auto cbeg = chrono::high_resolution_clock::now();
	    while (helper.next_record(i, fields)) {
		VehicleData vd(fields);
		cache.push_back(vd);
	    }
	    auto cent = chrono::high_resolution_clock::now();
	    auto len = chrono::duration_cast<chrono::milliseconds>(cent - cbeg).count();
	    count += 1;
	    Loger::log("已缓存-压缩车辆(" + to_string(count) + "/" + to_string(helper.csvfile_count()) + ")" + "\t用时" + to_string(len) + "ms"+ "  \t[" + helper.filename(i) + "])");
	}
    });
    
    thread thr3 ([&count, &helper, &cache]() {
	vector<string> fields;
	for (size_t i = 90; i < 120 && i < helper.csvfile_count(); ++i) {
	    auto cbeg = chrono::high_resolution_clock::now();
	    while (helper.next_record(i, fields)) {
		VehicleData vd(fields);
		cache.push_back(vd);
	    }
	    auto cent = chrono::high_resolution_clock::now();
	    auto len = chrono::duration_cast<chrono::milliseconds>(cent - cbeg).count();
	    count += 1;
	    Loger::log("已缓存-压缩车辆(" + to_string(count) + "/" + to_string(helper.csvfile_count()) + ")" + "\t用时" + to_string((len)) + "ms"+ "  \t[" + helper.filename(i) + "])");
	}
    });

    thr0.join();
    thr1.join();
    thr2.join();
    thr3.join();

    auto send = chrono::high_resolution_clock::now();
    cache.count();
    cache.statistic();
    cache.write_to_file("compressedData");
    auto timeUsed = std::chrono::duration_cast<chrono::milliseconds>(send - sbeg).count();
    Loger::log("共缓存" + to_string(helper.record_count()) + "条数据，共用时" + to_string(timeUsed) + "ms");
    if (timeUsed < 1000) timeUsed = 1000;
    Loger::log("平均速率:" + to_string(helper.record_count() * 1000 / (timeUsed)) + "条/秒");
    Loger::log("----------------------------------------压缩率测试结束-------------------------------------------------");
    Loger::log("-------------------------------------------------------------------------------------------------------");
}

int mysql_test(Cache& cache, CSVHelper& helper) {
    helper.split(helper.csvfile_count());
    size_t n = helper.csvfile_count();
    for (size_t i = 0; i < helper.csvfile_count(); ++i) {
	if (helper.filename(i).find("W1E-021") != std::string::npos) { n = i; }
    }

    if (n == helper.csvfile_count()) {
	Loger::err("没有找到测试车辆W1E-001");
	return 1;
    }

    std::shared_ptr<Vehicle> vehicle = cache.get_vehicle("W1E-001");
    if (vehicle == nullptr) {
	Loger::err ("get_vehicle W1E-001 fail");
	return 1;
    }

    sql::Driver* driver;
    sql::Connection* con;
    sql::Statement* stat;
    
    Loger::log("-------------------------------------------------------------------------------------------------------");
    Loger::log("----------------------------------Mysql性能对比测试开始------------------------------------------------");
    try {
	Loger::log("准备Mysql环境...");
	driver = get_driver_instance();
	if (driver == nullptr)
	    throw sql::SQLException("获取driver失败");
	con = driver->connect("localhost", "root", "3646");
	stat = con->createStatement();

	Loger::log("准备Mysql环境...完成.");
    }
    catch (sql::SQLException& e) {		/* handle exception: */
	Loger::log(string("Mysql环境准备失败") + e.what());
	return 1;
    }

    std::string sql;
    auto tbeg = chrono::high_resolution_clock::now();
    try {
	stat->execute("USE vehicles");
	std::vector<std::string> fields;

	Loger::log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>原始数据直接写入测试开始");
	Loger::log ("数据直接写入Mysql...");
	size_t count = 0;
	while (helper.next_record(n, fields)) {
	    try {
		sql = "INSERT INTO Origin VALUES ("
		    "'" + fields[0] + "',"      /* ID */
			+ fields[1] + ","       /* 时间 */
			+ fields[2] + ","       /* 经度 */
			+ fields[3] + ","       /* 纬度*/
			+ fields[4] + ","       /* 车门1 */
			+ fields[5] + ","       /* 车门2 */
			+ fields[6] + ","       /* 左转向灯 */
			+ fields[7] + ","       /* 右转向灯 */
			+ fields[8] + ","       /* 近光灯 */
			+ fields[9] + ","       /* 远光灯 */
			+ fields[10] + ","      /* 前雾灯 */
			+ fields[11] + ","      /* 刹车灯 */
			+ fields[12] + ","      /* 雨刷 */
			+ fields[13] + ","      /* 缓速器 */
			+ fields[14] + ","      /* 风扇 */
			+ fields[15] + ","      /* 跛行 */
			+ fields[16] + ","      /* 开门滑行 */
			+ fields[17] + ","      /* 急加速 */
			+ fields[18] + ","      /* 急减速 */
			+ fields[19] + ","      /* 空挡滑行 */
			+ fields[20] + ","      /* 油门过大 */
			+ fields[21] + ","      /* 空调制冷 */
			+ fields[22] +	    /* 通风 */
		")";
		stat->execute(sql);
		if (helper.record_count() - count >= 5000) {
		    Loger::log("写入Mysql:" + to_string(helper.record_count()) + "/61676条");
		    count = helper.record_count();
		}
	    }
	    catch (sql::SQLException& ex) {
	    }
	}
	Loger::log ("数据直接写入Mysql...完成");

	auto tend = chrono::high_resolution_clock::now();
	size_t wastOfMysql = chrono::duration_cast<chrono::milliseconds>(tend - tbeg).count();
	size_t speedOfMysql = helper.record_count() * 1000 / wastOfMysql;

	Loger::log("所用时间:" + to_string(wastOfMysql) + "ms");
	Loger::log("写入数据:" + to_string(helper.record_count()) + "条");
	Loger::log("平均速率:" + to_string(speedOfMysql) + "条/秒");
	Loger::log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>原始数据写入测试结束");

	Loger::log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>通过缓存写入Mysql测试结束");
	Loger::log ("通过缓存压缩将原始数据写入Mysql...");

	tbeg = chrono::high_resolution_clock::now();
	count = 0;
	try {
	    stat->execute("USE vehicles_compressed");
	    auto ttbeg = std::chrono::high_resolution_clock::now();
	    for (auto& t1 : vehicle->locationTimeline.timeline) {
		for (auto& t2: t1) {
		    try {
			count += 1;
			sql = std::string("INSERT INTO 位置 VALUES (")
			    + "'" + vehicle->id + "',"
			    + std::to_string(t2.st) + ","
			    + std::to_string(t2.sv.longitude) + ","
			    + std::to_string(t2.sv.latitude) +
			    ")";
			stat->execute(sql);
			sql = std::string("INSERT INTO 位置 VALUES (")
			    + "'" + vehicle->id + "',"
			    + std::to_string(t2.et) + ","
			    + std::to_string(t2.ev.longitude) + ","
			    + std::to_string(t2.ev.latitude) +
			    ")";
			stat->execute(sql);
		    }
		    catch (sql::SQLException& ex) {
			Loger::err(ex.what());
		    }
		}
	    }
	    auto ttend = std::chrono::high_resolution_clock::now();
	    auto wast = chrono::duration_cast<chrono::milliseconds>(ttend - ttbeg).count();
	    Loger::log("通过缓存写入Mysql\t位置\t(用时:" + to_string(wast) + "ms)");
	}
	catch (sql::SQLException& ex) {
	    Loger::log(ex.what());
	}

	vector<string> tableNames = {
	       "车门1", 
	       "车门2", 
	       "左转向灯", 
	       "右转向灯", 
	       "近光灯", 
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

	for (size_t i = 0; i < tableNames.size(); ++i) {
	    auto ttbeg = std::chrono::high_resolution_clock::now();
	    for (auto& tl : vehicle->generalTimelines[i].timeline) {
		for (auto& node : tl) {
		    try {
			count += 1;
			string sql = "INSERT INTO " + tableNames[i] + " VALUES (" 
			+ "'" + vehicle->id + "',"
			+ to_string(node.st) + ","
			+ to_string(node.et) + ","
			+ to_string(node.v) 
			+ ")";
			stat->execute(sql);
		    }
		    catch (sql::SQLException& e) {
			Loger::err (e.what());
		    }
		}
	    }
	    auto ttend = std::chrono::high_resolution_clock::now();
	    auto wast = chrono::duration_cast<chrono::milliseconds>(ttend - ttbeg).count();
	    Loger::log("通过缓存写入Mysql\t" + tableNames[i] + "\t(用时:" + to_string(wast) + "ms)");
	}

	tend = chrono::high_resolution_clock::now();
	auto wasteOfCache = chrono::duration_cast<chrono::milliseconds>(tend - tbeg).count();
	size_t speedOfCache = helper.record_count() * 1000 / wasteOfCache;
	Loger::log ("通过缓存写入到MYSQL完成, 用时:" + to_string(wasteOfCache) + "ms");
	Loger::log ("压缩前字段条数: " + to_string(helper.record_count() * (tableNames.size() + 3)) + "条数");
	Loger::log ("压缩后字段条数: " + to_string(count) + "条");
	Loger::log ("使用时间: " + to_string(wasteOfCache) + "ms");
	Loger::log ("平均速率: " + to_string(speedOfCache) + "条/秒");
	Loger::log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>通过缓存写入Mysql测试开始");

	char buffer[256];
        Loger::log			 ("|                            直接写入Mysql和通过缓存后写入Mysql吞吐量对比                     |");
	snprintf (buffer, sizeof(buffer), "|---------------------------------------------------------------------------------------------|");	Loger::log (buffer);
	snprintf (buffer, sizeof(buffer), "|                    |       直接写入数据库      |              通过缓存写入数据库            |");	Loger::log (buffer);
	snprintf (buffer, sizeof(buffer), "|---------------------------------------------------------------------------------------------|");	Loger::log (buffer);
	snprintf (buffer, sizeof(buffer), "|      数据条数      |       %8lu条          |%8lu条(压缩前)    |%8lu条(压缩后)   |", helper.record_count(), helper.record_count(), count); Loger::log (buffer);
	snprintf (buffer, sizeof(buffer), "|      所用时间      |       %8lums          |                 %8lums                 |",  wastOfMysql,  wasteOfCache); Loger::log (buffer);
	snprintf (buffer, sizeof(buffer), "|       吞吐量       |      %8lu条/秒        |               %8lu条/秒                |", speedOfMysql, speedOfCache); Loger::log (buffer);
	snprintf (buffer, sizeof(buffer), "|---------------------------------------------------------------------------------------------|");	Loger::log (buffer);
    }
    catch (sql::SQLException& ex) {
	Loger::err(ex.what());
	return 1;
    }
    Loger::log("----------------------------------Mysql性能对比测试结束------------------------------------------------");
    Loger::log("-------------------------------------------------------------------------------------------------------");
    return 0;
}

int multi_test(Cache& cache, CSVHelper& helper, const vector<pair<string, uint16_t>>& hosts) {
    vector<sockaddr_in> addrs;
    for (auto& host: hosts) {
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(host.second);
	inet_pton(AF_INET, host.first.c_str(), &addr.sin_addr.s_addr);
	addrs.push_back(addr);
    }

    size_t wast1, wast2, wast3;

    Loger::log("----------------------------------------扩展性测试开始-------------------------------------------------");
    Loger::log("-------------------------------------------------------------------------------------------------------");
    /* 一台节点 */
    {
	Loger::log (">>>>>>>>>>>>>>>>>>>>只向一台节点发送");
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	size_t size = 1024 * 1024 * 100;
	socklen_t len = sizeof (size);
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &size, len);
	int err = connect(sock, (sockaddr*)&addrs[0], (socklen_t)sizeof(addrs[0]));
	if (err == -1) {
	    Loger::err("连接到" +  hosts[0].first + "失败");
	    return 1;
	}

	helper.split(helper.csvfile_count());
	vector<string> fields;
	auto tb = chrono::high_resolution_clock::now();
	for (int n = 0; n < 30; ++n) {
	    while (helper.next_record(n, fields)) {
		VehicleData vd(fields);
		Package package = make_package(0, vd);
		Package encodedPack = encode_package(package);
		ssize_t sn = send (sock , encodedPack.data(), encodedPack.size(), 0);
		if (sn != (ssize_t)encodedPack.size()) {
		    Loger::log("发送失败");
		    return 1;
		}
	    }
	}

	close(sock);
	Loger::log("向一台节点发送完成");
	auto te = chrono::high_resolution_clock::now();
	wast1 = chrono::duration_cast<chrono::milliseconds>(te - tb).count();
	
	Loger::log("用时:" + to_string(wast1) + "ms");
    }

    size_t len = helper.record_count();

   
    /* 两台节点 */
    {
	Loger::log (">>>>>>>>>>>>>>>>>>>>同时向两台节点发送");
	helper.split(helper.csvfile_count());
	thread thr0 = thread ([&helper, &addrs, &hosts]() {
	    int sock = socket(AF_INET, SOCK_STREAM, 0);
	    size_t size = 1024 * 1024 * 100;
	    socklen_t len = sizeof (size);
	    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &size, len);
	    int err = connect(sock, (sockaddr*)&addrs[0], (socklen_t)sizeof(addrs[0]));
	    if (err == -1) {
		Loger::err("连接到" +  hosts[0].first + "失败");
		return 1;
	    }

	    Loger::log("开始向节点0发送...");
	    vector<string> fields;
	    for (int n = 0; n < 15; ++n) {
		while (helper.next_record(n, fields)) {
		    VehicleData vd(fields);
		    Package package = make_package(0, vd);
		    Package encodedPack = encode_package(package);
		    ssize_t sn = send (sock , encodedPack.data(), encodedPack.size(), 0);
		    if (sn != (ssize_t)encodedPack.size()) {
			Loger::log("发送失败");
			return 1;
		    }
		}
	    }
	    close(sock);
	    Loger::log ("向节点0发送完毕");
	    return 0;
	});

	thread thr1 = thread ([&helper, &addrs, &hosts]() {
	    int sock = socket(AF_INET, SOCK_STREAM, 0);
	    size_t size = 1024 * 1024 * 100;
	    socklen_t len = sizeof (size);
	    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &size, len);
	    int err = connect(sock, (sockaddr*)&addrs[1], (socklen_t)sizeof(addrs[1]));
	    if (err == -1) {
		Loger::err("连接到" +  hosts[1].first + "失败");
		return 1;
	    }

	    Loger::log("开始向节点1发送...");
	    vector<string> fields;
	    for (int n = 15; n < 30; ++n) {
		while (helper.next_record(n, fields)) {
		    VehicleData vd(fields);
		    Package package = make_package(0, vd);
		    Package encodedPack = encode_package(package);
		    ssize_t sn = send (sock , encodedPack.data(), encodedPack.size(), 0);
		    if (sn != (ssize_t)encodedPack.size()) {
			Loger::log("发送失败");
			return 1;
		    }
		}
	    }
	    close(sock);
	    Loger::log("开始向节点1发送...完毕");
	    return 0;
	});

	auto tb = chrono::high_resolution_clock::now();
	thr0.join();
	thr1.join();
	auto te = chrono::high_resolution_clock::now();
	wast2 = chrono::duration_cast<chrono::milliseconds>(te - tb).count();
	Loger::log("同时向两台节点发送完毕");
	Loger::log ("用时" + to_string(wast2) + "ms");
    }

    /* 三台节点 */
    {
	Loger::log (">>>>>>>>>>>>>>>>>>>>同时向三台节点发送");
	helper.split(helper.csvfile_count());
	thread thr0 = thread ([&helper, &addrs, &hosts]() {
	    int sock = socket(AF_INET, SOCK_STREAM, 0);
	    size_t size = 1024 * 1024 * 100;
	    socklen_t len = sizeof (size);
	    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &size, len);

	    int err = connect(sock, (sockaddr*)&addrs[0], (socklen_t)sizeof(addrs[0]));
	    if (err == -1) {
		Loger::err("连接到" +  hosts[0].first + "失败");
		return 1;
	    }

	    Loger::log("开始向节点0发送...");
	    vector<string> fields;
	    for (int n = 0; n < 5; ++n) {
		while (helper.next_record(n, fields)) {
		    VehicleData vd(fields);
		    Package package = make_package(0, vd);
		    Package encodedPack = encode_package(package);
		    ssize_t sn = send (sock , encodedPack.data(), encodedPack.size(), 0);
		    if (sn != (ssize_t)encodedPack.size()) {
			Loger::log("发送失败");
			return 1;
		    }
		}
	    }
	    close(sock);
	    Loger::log("开始向节点0发送完成");
	    return 0;
	});

	thread thr1 = thread ([&helper, &addrs, &hosts]() {
	    int sock = socket(AF_INET, SOCK_STREAM, 0);
	    size_t size = 1024 * 1024 * 100;
	    socklen_t len = sizeof (size);
	    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &size, len);
	    int err = connect(sock, (sockaddr*)&addrs[1], (socklen_t)sizeof(addrs[1]));
	    if (err == -1) {
		Loger::err("连接到" +  hosts[1].first + "失败");
		return 1;
	    }

	    Loger::log("开始向节点1发送...");
	    vector<string> fields;
	    for (int n = 10; n < 15; ++n) {
		while (helper.next_record(n, fields)) {
		    VehicleData vd(fields);
		    Package package = make_package(0, vd);
		    Package encodedPack = encode_package(package);
		    ssize_t sn = send (sock , encodedPack.data(), encodedPack.size(), 0);
		    if (sn != (ssize_t)encodedPack.size()) {
			Loger::log("发送失败");
			return 1;
		    }
		}
	    }
	    close(sock);
	    Loger::log("开始向节点1发送完成");
	    return 0;
	});

	thread thr2 = thread ([&helper, &addrs, &hosts]() {
	    int sock = socket(AF_INET, SOCK_STREAM, 0);
	    size_t size = 1024 * 1024 * 100;
	    socklen_t len = sizeof (size);
	    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &size, len);
	    int err = connect(sock, (sockaddr*)&addrs[2], (socklen_t)sizeof(addrs[2]));
	    if (err == -1) {
		Loger::err("连接到" +  hosts[2].first + "失败");
		return 1;
	    }

	    Loger::log("开始向节点2发送...");
	    vector<string> fields;
	    for (int n = 20; n < 25; ++n) {
		while (helper.next_record(n, fields)) {
		    VehicleData vd(fields);
		    Package package = make_package(0, vd);
		    Package encodedPack = encode_package(package);
		    ssize_t sn = send (sock , encodedPack.data(), encodedPack.size(), 0);
		    if (sn != (ssize_t)encodedPack.size()) {
			Loger::log("发送失败");
			return 1;
		    }
		}
	    }
	    close(sock);
	    Loger::log("开始向节点2发送完成");
	    return 0;
	});

	auto tb = chrono::high_resolution_clock::now();
	thr0.join();
	thr1.join();
	thr2.join();
	auto te = chrono::high_resolution_clock::now();
	wast3 = chrono::duration_cast<chrono::milliseconds>(te - tb).count();
	Loger::log("同时向三台节点发送完毕");
	Loger::log ("用时" + to_string(wast3) + "ms");
    }

    Loger::log ("节点发送数据测试完毕");

    size_t recordCount = len;
    char buffer[256];
    snprintf (buffer, sizeof(buffer), "|                                扩展性测试结果                               |"); Loger::log (buffer);
    snprintf (buffer, sizeof(buffer), "|-----------------------------------------------------------------------------|"); Loger::log(buffer);
    snprintf (buffer, sizeof(buffer), "|              |       一台节点       |      两台节点     |      三台节点     |"); Loger::log (buffer);
    snprintf (buffer, sizeof(buffer), "|-----------------------------------------------------------------------------|"); Loger::log(buffer);
    snprintf (buffer, sizeof(buffer), "|   数据条数   |      %8lu条      |   %8lu条      |    %8lu条     |", recordCount, recordCount, recordCount); Loger::log(buffer);
    snprintf (buffer, sizeof(buffer), "|   用时       |     %8lums       |   %8lums      |   %8lums      |", wast1, wast2, wast3); Loger::log(buffer);
    snprintf (buffer, sizeof(buffer), "|   输出速率   | %8lu条/秒        | %8lu条/秒     | %8lu条/秒     |", recordCount * 1000 / wast1, recordCount * 1000 / wast2, recordCount * 1000 / wast3); Loger::log (buffer);
    snprintf (buffer, sizeof(buffer), "|-----------------------------------------------------------------------------|"); Loger::log(buffer);

    Loger::log("----------------------------------------扩展性测试结束-------------------------------------------------");
    Loger::log("-------------------------------------------------------------------------------------------------------");
    return 0;
}

int main(int argc, char *argv[]) {
    Cache cache;
    CSVHelper helper("sorted");
    CSVHelper helper1("equal");
    vector<pair<string, uint16_t>> hosts;
    hosts.push_back (make_pair<string, uint16_t>("127.0.0.1", 8080));
    hosts.push_back (make_pair<string, uint16_t>("127.0.0.1", 8081));
    hosts.push_back (make_pair<string, uint16_t>("127.0.0.1", 8082));
    //cache_test(cache, helper);
    //Loger::log ("按Enter键继续");
    //"getchar();
    //"mysql_test(cache, helper);
    Loger::log ("按Enter键继续");
    getchar();
    multi_test(cache, helper1, hosts);
    Loger::log ("按Enter键继续");
    getchar();
    Loger::log ("测试结束");
    Loger::wait();

    return 0;
}
