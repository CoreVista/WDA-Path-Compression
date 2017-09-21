/*
 * =====================================================================================
 *
 *       Filename:  data.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/21/2017 11:31:09 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include "data.h"

/*-----------------------------------------------------------------------------
 *  全局定义
 *-----------------------------------------------------------------------------*/

Package encode_package(const Package& pkg) {
    Package p;
    p.push_back(PackageMonitor::flagStart);
    for (auto pc = pkg.begin(); pc != pkg.end(); ++pc) {
	if (*pc == PackageMonitor::flagStart || *pc == PackageMonitor::flagEnd || *pc == PackageMonitor::flagChange)
	    p.push_back(PackageMonitor::flagChange);
	p.push_back(*pc);
    }
    p.push_back(PackageMonitor::flagEnd);
    return p;
}

Package decode_package(const Package& pack) {
    Package tmp;
    for (size_t i = 1; i < pack.size() - 1; ++i) {
	if (pack[i] == PackageMonitor::flagChange)
	    i += 1;
	 tmp.push_back(pack[i]);
    }
    return tmp;
}

Package make_package (const std::initializer_list<byte>& lst) {
    return Package(lst); 
}

Package make_package(uint64_t tokenId, VehicleData& v) {
    std::vector<unsigned char> tmp;
    tmp.push_back(System::is_big_endian() ? 1 : 0);
    unsigned char *it = reinterpret_cast<unsigned char *>(&tokenId);
    for (size_t i = 0; i < sizeof(tokenId); ++i) 
	tmp.push_back (*it++);
    v.push_back_to(std::back_inserter(tmp));
    return tmp;
}

std::pair<uint64_t, uint64_t> 
    generate_index_from_vehicle_id(const std::string& id) {
    uint64_t key1 = 0, key2 = 0;
    auto it = id.rbegin();
    for (size_t loop = 0; loop < SIZE_OF_ID; ++loop) {
	byte c = (it != id.rend()) ? (*it++) : '\0';
	if (loop < sizeof(key1)) {
	    key1 <<= 8;
	    key1 |= c;
	}
	else {
	    key2 <<= 8;
	    key2 |= c;
	}
    }
    return std::make_pair(key1, key2);
}
/*-----------------------------------------------------------------------------
 *  PackageMonitor定义
 *-----------------------------------------------------------------------------*/
int PackageMonitor::put (unsigned char *buffer, size_t size, std::list<std::vector<unsigned char>>& lists) {
    unsigned char d;
    bool ok = true;
    for (size_t i = 0; ok && i < size; i ++) {
	d = buffer[i];
	switch (stat) {
	case START:
	    if (d != flagStart)
		ok = false;
	    else
		stat = END;
	    break;
	    
	case END:
	    if (d == flagChange)
		stat = CHANGE;
	    else if (d == flagEnd) {
		lists.push_back(std::move(data));
		stat = Stat::START;
	    }
	    else
		data.push_back (d);
	    break;

	case CHANGE:
	    if (d == flagStart || d == flagEnd || d == flagChange) {
		data.push_back(d);
		stat = Stat::END;
	    }
	    else
		return -1;
	    break;

	default:
	    Loger::warning("Unreachable");
	    break;
	}
    }

    if (!ok) {
	reset();
	return -1;
    }
    return lists.size();
}

/*-----------------------------------------------------------------------------
 *  Message 定义
 *-----------------------------------------------------------------------------*/
const unsigned char PackageMonitor::flagStart;
const unsigned char PackageMonitor::flagEnd;
const unsigned char PackageMonitor::flagChange;

Message::Message(int sock, uint64_t sid, Package&& package)
    :package(package), sock(sock), sid(sid) {
    if (package.size() < 9)
	throw std::length_error("package不足8个字节，怎么让我愉快的提取tokenID");
    tokenID = *((uint64_t*)(package.data() + 1));
    if (package[0])	    /* 如果对方是大端，还需要进行大端到小端的转换 */
	tokenID = ntohll(tokenID);
}

/*-----------------------------------------------------------------------------
 *  VehicleData 定义
 *-----------------------------------------------------------------------------*/
VehicleData::VehicleData (const Message& msg) {
    const byte* beg = msg.package_begin();
    const byte* end = msg.package_end();
    if (end - beg != size()) 
	throw std::length_error("从Message到VehicleData转换的长度不符合");
    const byte *it = beg;
    memcpy(&when, it, sizeof(when));
    it += sizeof(when);
    memcpy(&longitude, it, sizeof(double));
    it += sizeof(double);
    memcpy(&latitude, it, sizeof(double));
    it += sizeof(double);
    id = reinterpret_cast<const char*>(it);
    it += SIZE_OF_ID;
    memcpy(bits, it, sizeof(bits));
    generate_index();
}

VehicleData::VehicleData (const std::vector<std::string> & fields) {
    for (size_t i = 0; i < fields.size(); ++i) {
	switch (i) {
	case 0:
	    id = fields[0];
	    generate_index();
	    break;
	case 1:		
	    when = std::strtoul(fields[1].c_str(), nullptr, 10);
	    break;
	case 2:
	    longitude = std::strtod(fields[2].c_str(), nullptr);
	    break;
	case 3:
	    latitude = std::strtod(fields[3].c_str(), nullptr);
	    break;
	default:
	    bits[i - 4] = (fields[i] == "0") ? 0 : 1;
	    break;
	}
    }
}

void VehicleData::push_back_to(std::back_insert_iterator<std::vector<unsigned char>> to) {
    unsigned char *from = nullptr;
    from = reinterpret_cast<byte*>(&when);
    for (size_t i = 0; i < sizeof(when); ++i)
	*to = *(from ++);

    from = reinterpret_cast<byte*>(&longitude);
    for (size_t i = 0; i < sizeof(longitude); ++i)
	*to = *(from ++);

    from = reinterpret_cast<byte*>(&latitude);
    for (size_t i = 0; i < sizeof(latitude); ++i)
	*to = *(from ++);
    
    for (size_t i = 0; i < SIZE_OF_ID; ++i)
	*to = i < id.size() ? id[i] : '\0';

    from = reinterpret_cast<byte*>(&bits);
    for (size_t i = 0; i < sizeof(bits); ++i)
	*to = *(from ++);
}

void VehicleData::generate_index() {
    key1 = key2 = 0;
    auto it = id.rbegin();
    for (size_t loop = 0; loop < SIZE_OF_ID; ++loop) {
	byte c = (it != id.rend()) ? (*it++) : '\0';
	if (loop < sizeof(key1)) {
	    key1 <<= 8;
	    key1 |= c;
	}
	else {
	    key2 <<= 8;
	    key2 |= c;
	}
    }
}

/*-----------------------------------------------------------------------------
 *  Vehicle 定义
 *-----------------------------------------------------------------------------*/
void Vehicle::push_back (const VehicleData& v) {
    mtx.lock();
    Location loc;
    loc.longitude = v.longitude; 
    loc.latitude = v.latitude;
    this->locationTimeline.push_back(loc, v.when);
    for (size_t i = 0; i < sizeof(v.bits) / sizeof(v.bits[0]); ++i) {
	generalTimelines[i].push_back(v.bits[i], v.when);
    }
    mtx.unlock();
}

std::string Vehicle::to_string() {
    std::ostringstream ss;
    ss << id << ":\n";
    ss << "Location:";
    for (auto v : locationTimeline.timeline)
	for (auto vv : v)
	    ss << vv.to_string() << " ";
    ss << "\n";
    for (auto& v: generalTimelines) {
	ss << "V:";
	for (auto& vv: v.timeline) {
	    for (auto& vvv : vv) {
		ss << vvv.to_string() << " ";
	    }
	}
	ss << "\n";
    }
    return ss.str();
}

using namespace std;
void Vehicle::write_to_file(string outDir) {
    if (outDir.empty())
	outDir = ".";
    string outPath = outDir + "/" + id;
    mkdir(outDir.c_str(), 0776);
    mkdir(outPath.c_str(), 0776);
    errno = 0;
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

    string of = outPath + "/" + titles[0] + ".csv";
    ofstream ofs(of, ios_base::trunc);
    if (!ofs) {
	Loger::err("打开文件" + of);
	return;
    }

    ofs << "时间,经度,纬度" << endl;
    for (auto& locn1 : locationTimeline.timeline) {
	for (auto &locn : locn1 ) {
	    ofs << locn.st << "," << locn.sv.to_string() << endl;
	    ofs << locn.et << "," << locn.ev.to_string() << endl;
	}
    }
    ofs.close();

    int i = 1;
    for (auto& gv : generalTimelines) {
	of = outPath + "/" + titles[i++] + ".csv";
	ofs.open(of, ios_base::trunc);
	if (!ofs) {
	    Loger::err("打开文件" + of);
	    return;
	}
	ofs << "起始时间,结束时间,状态" << std::endl;
	for (auto& v : gv.timeline) {
	    for (auto& vv : v) {
		ofs << vv.st << "," << vv.et << "," << (int)vv.v << endl;
	    }
	}
	ofs.close();
    }
}
