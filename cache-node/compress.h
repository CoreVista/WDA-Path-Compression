/*
 * =====================================================================================
 *
 *       Filename:  compress.h
 *
 *    Description: 
 *
 *        Version:  1.0
 *        Created:  05/24/2017 05:09:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __INCLUDE_COMPRESS
#define __INCLUDE_COMPRESS

#include <iostream>
#include <array>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <list>
#include <sstream>
#include <cmath>
#include "../com/loger.h"
#include "../com/system.h"

const double EARTH_RADIUS = 6378.137; 
const double PI = 3.1415926535898;

struct Location {
    double latitude, longitude;
    std::string to_string() {
	std::ostringstream oss;
    
	oss << std::setprecision(10) << longitude << "," << std::setprecision(10)  << latitude;
	return oss.str();
    }
};

struct LocationNode {
    using ValueType = Location;

    time_t st, et;
    Location sv, ev;
    LocationNode():st(-1), et(-1) {}
    bool isNew() { return st == -1; }

    std::string to_string() {
	std::ostringstream oss;
	oss << "{" << st  << ":" << sv.to_string() << ";" 
	    << et << ":" << ev.to_string() << "}";
	return oss.str();
    }
};

template <typename VT>
struct GeneralNode {
    using ValueType = VT;
    time_t st, et;
    ValueType v;
    GeneralNode(): st(-1), et(-1) {}
    bool isNew() {return st == -1;}

    std::string to_string() {
	std::ostringstream oss;
	oss << "{" << st << "," << et  << ":"  << (int)v << "}";
	return oss.str();
    }
};

template <typename NodeType>
struct Timeline {
    ssize_t index = 0;
    std::vector<std::vector<NodeType>> timeline;
    virtual void push_back (const typename NodeType::ValueType& v, time_t t) = 0;

    bool isEmpty() {
	return timeline.size() == 0 || (timeline.size() == 1 && timeline.back().size() == 0); 
    }

    bool hasFreeSpace() { 
	return timeline.size() > 0 && timeline.back().capacity() - timeline.back().size() > 0; 
    }
    
    NodeType& last() {
	return timeline.back().back(); 
    }

    NodeType& alloc() {
	if (!hasFreeSpace()) {
	    timeline.emplace_back();
	    timeline.back().reserve(1024);
	}
	timeline.back().emplace_back();
	return last();
    }

    NodeType& lastOrAlloc() {
	return isEmpty() ? alloc() : last();
    }

    size_t size() {
	size_t sum = 0;
	for (auto& t : timeline) {
	    sum += t.size();
	}
	return sum;
    }
};

struct LocationTimeline: public Timeline<struct LocationNode> {
    double px, py;                          /* 上一次的x,y */
    double ppx, ppy;
    int driveAngle;
    bool isDriveVectorOk = false;

    virtual void push_back (const Location& loc, time_t t) override {
	using namespace std;
	double x = (loc.latitude + 90) / 180.0 * (PI * EARTH_RADIUS) * 1000;
	double y = (loc.longitude / 180) * (PI * EARTH_RADIUS) * 1000;

	LocationNode& node = lastOrAlloc();
	if (node.isNew()) {
	    node.st = node.et = t;
	    node.sv = node.ev = loc;
	    ppx = px = x;
	    ppy = py = y;
	    return;
	}
	
	double dis = sqrt (pow(py - y, 2) + pow(px - x, 2)); /* 本次的位置和上一次位置距离 */
	// 如果本次和上次的位置的距离小于20米，则只更新时间
	if (dis <= 20) {
	    node.et = t;
	    return ;
	}

	if (!isDriveVectorOk) {
	    node.ev = loc;
	    driveAngle = angle(px, py, x, y);
	    isDriveVectorOk = true;
	    px = x;
	    py = y;
	    return ;
	}

	int can = angle (px, py, x, y);	    /* 本次和上次位置形成的夹角 */
	int dan;                            /* 本次和上次位置所形成夹角与行使方向的夹角 */
	{
	    dan = abs(driveAngle - can);
	    if (dan> 180) 
		dan = 360 % dan;
	}

	if (dan > 90) {	  /* 如果夹角大于90度，说明在转弯 */
	    LocationNode& node = alloc();
	    node.sv = node.ev = loc;
	    node.st = node.et = t;
	    isDriveVectorOk = false;
	    ppx = px = x;
	    ppy = py = y;
	    return ;
	}
	
	dis = sqrt (pow(ppy - y, 2) + pow(ppx - x, 2)); /* 本次的位置和上一次位置距离 */
	double disToVec = dis * sin(dan * PI / 180.0);
	if (disToVec > 150) { /* 如果本次位置距离位置向量大于150米则新建一个存储节点 */
	    LocationNode& node = alloc();
	    node.sv = node.ev = loc;
	    node.st = node.et = t;
	    isDriveVectorOk = false;
	    ppx = px = x;
	    ppy = py = y;
	    return ;
	}
 
	node.ev = loc;
	node.et = t;
	px = x;
	py = y;
    }

    // 给定坐标点和上一次坐标点之间的夹角
    inline double angle (double px, double py, double x, double y) {
	double angle;
	if (px == x)  angle = 90;
	else if (py == y)  angle = 0;
	else angle = std::atan((y - py) / (x - px)) / PI * 180;

	if (angle == 0) {
	    if (px > x) angle = 180;
	} 
	else if (angle == 90) {
	    if (py > y) angle = 270; 
	}
	else if (angle > 0) {
	    if (px > x) angle += 180;
	}
	else if (angle < 0) {
	    angle += ((py < y) ? 180 : 360);
	}
	return angle;
    }
};

template <typename VT = char>
struct GeneralTimeline : public Timeline<GeneralNode<VT>> {
    using ValueType = VT;

    virtual void push_back(const ValueType& v, time_t t) override {
	GeneralNode<ValueType>& node = Timeline<GeneralNode<ValueType>>::lastOrAlloc();
	if (node.isNew()) {
	    node.v = v;
	    node.st = node.et = t;
	    return;
	}

	if (node.v != v) {
	    auto& node = Timeline<GeneralNode<ValueType>>::alloc();
	    node.v = v;
	    node.st = node.et = t;
	    return;
	}
	node.et = t;
    }
};

#endif
