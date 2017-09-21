#ifndef __INCLUDE_DATA
#define __INCLUDE_DATA

#include <string>
#include <stdexcept>
#include <vector>
#include <list>
#include <netdb.h>
#include <bitset>
#include <sstream>

#include "../com/loger.h"
#include "../com/system.h"
#include "../com/error.h"
#include "../com/extension.h"
#include "compress.h"

using byte = unsigned char;
using Package = std::vector<byte>;

#define SIZE_OF_ID  16

/*-----------------------------------------------------------------------------
    *  PackageMonior 
 *-----------------------------------------------------------------------------*/

/*
 * PackageMonitor 产生已经解码的Package
 */
class PackageMonitor {
/*
     * Package 格式描述
     * Package : 0 1 2 3 4 5 6 7 8 9 ....
     *           | | ----------- | | ---
     *           端      Token       Data
     *           0=Small
     *           1=Big
     *
     * --  转义描述 --
     *  0xaa -> 起始
     *  0xbb -> 结束
     *  0xcc -> 转义
     *  0xaa -> 0xcc 0xaa
     *  0xbb -> 0xcc 0xbb
     *  0xcc -> 0xcc 0xcc
 */
public:
    enum Stat { START, END, CHANGE };
    static const unsigned char flagStart  = 0xaa;
    static const unsigned char flagEnd	  = 0xbb;
    static const unsigned char flagChange = 0xcc;

    Stat stat = START;
    Package data;

public:
    int put (unsigned char *buffer, size_t size, std::list<std::vector<unsigned char>>& lists);
    void reset() { stat = Stat::START; data.clear(); }
};

/*-----------------------------------------------------------------------------
 *  Message 
 *-----------------------------------------------------------------------------*/
struct Message {
private:
    Package package;	//从第9字节起才是VehicleData

public:
    /* ID */
    int		sock;
    uint64_t	sid;
    uint64_t	tokenID;				    /* 令牌ID，默认为0 */
    time_t	time = std::time(nullptr);

public:
    Message(int sock, uint64_t sid, Package&& package);

public:
    inline const byte* package_begin() const { return package.data() + 9; }
    inline const byte* package_end() const { return package.data() + package.size(); }
};

/*-----------------------------------------------------------------------------
 *  VehicleData
 *-----------------------------------------------------------------------------*/
struct VehicleData {
    uint64_t	key1, key2;                    /* 由车牌号生成的二级索引 */
    std::string	id;                         /* 车牌号 */
    double	longitude;                  /* 经度 */
    double	latitude;                   /* 纬度 */
    /*  
        字段（原始CSV编号）
	- id
        车辆编号(3)
	- 
        上报时间 (4)
	- longtitude
        经度(7)
	- latitude
        纬度(8)
        - bits
        0. 车门1(17)
        1. 车门2(18)
        2. 左转向灯(19)
        3. 右转向灯(20)
        4. 光灯(21)
        5. 远光灯(22)
        6. 前雾灯(23)
        7. 刹车灯(26)
        8. 雨刷(28)
        9. 缓速器(29)
        10. 刹车信号(31)
        11. 风扇开关(33)
        12. 跛行状态(61)
        13. 开门滑行(62)
        14. 急加速(64)
        15. 急减速(65)
        16. 空档滑行(66) 
        17. 油门过大(67)
        18. 空调制冷(72)
	19. 通风 */
    char	bits[20];                   /* 状态信息 */
    time_t	when;                       /* 数据生成时间 */

    /* 默认构造 */
    VehicleData():longitude(0), latitude(0), bits{0} {}

    /* 通过接收到的消息构建msg */
    VehicleData (const Message& msg);

    /*  由从文件读取的字段数组构造 */ 
    VehicleData (const std::vector<std::string> & fields);

    /* 将自己的内容追加到back_insert_interator所指的地方 */
    void push_back_to(std::back_insert_iterator<std::vector<unsigned char>> to);

    /* 获取一条VehicleData应该在一个包中所占的的尺寸 */
    ssize_t size() { return sizeof(when) + sizeof(longitude) + sizeof(latitude) + SIZE_OF_ID + sizeof(bits); } 

    /* 根据id生成两个整形key存放到key1key2处 */
    void generate_index();
};

/*-----------------------------------------------------------------------------
 *  Vehicle
 *-----------------------------------------------------------------------------*/
struct Vehicle {
    Mutex mtx;
    std::string id;
    LocationTimeline locationTimeline;
    std::array<GeneralTimeline<char>, sizeof(VehicleData().bits)> generalTimelines;
    
    Vehicle(const std::string& id):id(id) {}
    void push_back (const VehicleData& v);
    std::string to_string();
    void write_to_file(std::string outDir);
};

Package encode_package(const Package& pkg);
Package decode_package(const Package& pack);
Package make_package (const std::initializer_list<byte>& lst);
Package make_package(uint64_t tokenId, VehicleData& v); // 按照tokenID和VehicleData创建一个package

std::pair<uint64_t, uint64_t> 
    generate_index_from_vehicle_id(const std::string& id);

#endif //INCLUDE_DATA
