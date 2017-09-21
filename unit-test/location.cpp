
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <list>
#include <cmath>

const double EARTH_RADIUS = 6378.137; 
const double PI = 3.1415926535898;

struct Location {
public:
    double latitude, longitude;
    double _x, _y;

public:
    Location (double latitude, double longitude):
        latitude(latitude), longitude(longitude), 
        _x((latitude + 90) / 180.0 * (PI * EARTH_RADIUS) * 1000), 
        _y((longitude / 180) * (PI * EARTH_RADIUS) * 1000) {  
    }

    const Location& operator=(const Location& loc) {
        latitude = loc.latitude;
        longitude = loc.longitude;
        _x = loc._x;
        _y = loc._y;
        return *this;
    }
};

std::ostream& operator << (std::ostream& os, const Location& loc) {
    os << "{" << loc.latitude << ", " << loc.longitude << "}";
    return os;
}

template <typename ValueType>
class CompressedField {
public:
    std::vector<ValueType> compressedData;

public:
    virtual void push_back(const ValueType& v) = 0; 
};

class CompressedLocation: public CompressedField<Location> {
private:
    const double PI = 3.1415926535898;
    bool drive_vector_ok = false;

public:
    int driverAngle;
    Location origin = Location (0.0, 0);

    virtual void push_back(const Location& loc) override {
        using namespace std;

        static size_t seid = 0;
        seid += 1;
        
        // 如果还没有一条，则只存储其他什么都不做
        if (compressedData.size() == 0) {
            compressedData.push_back(loc);
            /* cout << seid << ". " << "第一条位置信息，仅存储" << endl; */
            return;
        }
   
        double dis = distance_between_points(loc, compressedData.back());
        /* cout << seid << ". 与上一条位置的距离:" << dis << endl; */
        // 如果当前位置与上一次上传位置距离小于等于6米说明车没动，直接忽略
        if (dis <= 20) {
            /* cout << seid << ". " << "由于与上次的小于20米，忽略之" << endl; */
            return;
        }

        // 如果位置向量还没建立好，则建立行驶向量
        if (!drive_vector_ok) {
            compressedData.push_back(loc);
            build_drive_vector();
            /* cout << seid << ". 车辆新行驶方向：" << driverAngle << endl; */
            return;
        }

        double can = angle (compressedData.back(), loc);
        double dan = difangle(can, driverAngle);
        double disToVec = distance_to_drive_vector(loc);

        /* cout << seid << ":" << "行驶方向:" << can << "\t行驶向量:" << driverAngle << "\t差值:" << dan */ 
            /* << "\t行驶向量距离差:" << disToVec << std::endl; */

        /* cout << seid << ". " << "当前行驶角度为:" << can << ", 与车辆行驶角度差" << dan << endl; */
        if (dan > 90) {
            /* cout << seid << ". " << "当前行驶角度，与行驶向量角度大于90度，故新建行驶向量" << endl; */
            compressedData.push_back(loc);
            build_drive_vector();
            return;
        }

        /* cout << seid << ". " << "当前位置与行驶向量的距离" << disToVec << endl; */
        if (disToVec < 150) {
            compressedData.back() = loc;
            /* cout << seid << ". " << "该位置替换了最后一个位置" << endl; */
        }
        else {
            drive_vector_ok = false;
            compressedData.push_back(loc);
            /* cout << seid << ". " << "当前位置离行驶距离太远，需要新建行驶向量" << endl; */
        }
    }

    
    inline double rad(double v) { return v * PI / 180; }

    void build_drive_vector() {
        using namespace std;
        auto& t = *compressedData.rbegin();
        auto& o = *(compressedData.rbegin() + 1);
        origin = o;
        driverAngle = angle(o, t);
        drive_vector_ok = true;
        /* cout << "新的行驶向量：" << driverAngle << endl; */
    }

    double distance_between_points2 (const Location& loc1, const Location& loc2) {
        double x1 = rad(loc1.latitude);
        double y1 = rad(loc1.longitude);
        double x2 = rad(loc2.latitude);
        double y2 = rad(loc2.longitude);

        double dx = x1 - x2;
        double dy = y1 - y2;

        double s = 2 * asin(sqrt(pow(sin(dx / 2), 2) + cos(x1) * cos(x2) * pow(sin(dy / 2), 2)));
        s = s * EARTH_RADIUS * 1000;

        return s;
    }

    inline double distance_between_points(const Location& loc1, const Location& loc2) {
        return sqrt (pow(loc2._y - loc1._y, 2) + pow(loc2._x - loc1._x, 2));
    }

    inline double distance_to_drive_vector(const Location& loc) {
        using namespace std;

        double dis = distance_between_points(origin, loc);
        double an = rad(angle (origin, loc) - driverAngle);
        return abs(dis * sin (an));
    }

    inline int angle (const Location& o, const Location& t) {
        double an;
        if (o._x == t._x)  an = 90;
        else if (o._y == t._y)  an = 0;
        else an = std::atan((t._y - o._y) / (t._x - o._x)) / PI * 180;

        if (an == 0) {
            if (o._x > t._x) an = 180;
        } 
        else if (an == 90) {
            if (o._y > t._y) an = 270; 
        }
        else if (an > 0) {
            if (o._x > t._x) an += 180;
        }
        else if (an < 0) {
            an += ((o._y < t._y) ? 180 : 360);
        }
        return an;
    }

    inline int difangle(int a1, int a2) {
        int d = abs(a1 - a2);
        if (d > 180) 
            d = 360 % d;
        return d;
    }
};

class CompressedDouble: public CompressedField<double> {
private:
    double precision;
    double precision_1;

public:
    CompressedDouble (double precision):precision(precision), precision_1(1 / precision) {
    }

    virtual void push_back(const double& d) override {
        if (compressedData.size() == 0) {
            compressedData.push_back(d);
            return;
        }

        int pv = static_cast<int>(compressedData.back() * precision_1);
        int cv = static_cast<int>(d * precision_1);

        if (pv != cv)
            compressedData.push_back(d);
    }
};

class CompressedInt: public CompressedField<int> {
private:
    int precision;

public:
    CompressedInt (int precision):precision(precision) {
    }

    virtual void push_back(const int& d) override {
        if (compressedData.size() == 0) {
            compressedData.push_back(d);
            return;
        }

        int pv = compressedData.back() / precision;
        int cv = d / precision; 

        if (pv != cv)
            compressedData.push_back(d);
    }
};

class CompressedBool: public CompressedField<bool> {
public:
    CompressedBool () {
    }

    virtual void push_back(const bool& b) override {
        if (compressedData.size() == 0) {
            compressedData.push_back(b);
            return;
        }

        int pv = compressedData.back(); 
        if (pv != b)
            compressedData.push_back(b);
    }
};

// 暂时没有用到
class Compress {
private:
    CompressedLocation locations;
    CompressedBool door1;
    CompressedBool door2;
    CompressedBool leftLight;
    CompressedBool rightLight;
    CompressedBool nearLight;
    CompressedBool farLight;
    CompressedBool qianWuDeng;
    CompressedBool houWuDeng;
    CompressedBool weiZhiDeng;
    CompressedBool shaCheDeng;
    CompressedBool shouShaXinHao;
    CompressedBool yuShua;
    CompressedBool huanSuQi;
    CompressedBool liHeQi;
    CompressedBool jiaoShaXinHao;
    CompressedBool bianSuXiang;
    CompressedBool dianCiFengShan;
    CompressedDouble xvDianChiDianYa = CompressedDouble(1);
    CompressedDouble dianChiZuSOC = CompressedDouble(1);
    CompressedInt dianChiZuYunXuZuiDaDianLiu = CompressedInt(1);

public:
    void compress(const std::string& , int index) {
        switch (index) {
            
        }
    }
};

void compress_file(const std::string& ) {
    using namespace std;
}

int _main(int , char *[]) {
    using namespace std;
    CompressedLocation compressedLoc;

    while (false) {
        cout << "请输入行驶向量Origin以及行驶方向:" << flush;
        cin >> compressedLoc.origin._x >> compressedLoc.origin._y >> compressedLoc.driverAngle;
        cout << "请输入要计算距离的坐标:" << flush;
        Location loc = Location(0, 0);
        cin >> loc._x >> loc._y;
        cout << compressedLoc.distance_to_drive_vector(loc) << endl;
    }

    if (false) {
        std::cout << compressedLoc.angle(Location(1, 1), Location(0, 0)) << std::endl;
        std::cout << compressedLoc.angle(Location(0, 0), Location(1, 1)) << std::endl;
        std::cout << compressedLoc.angle(Location(1, 2), Location(2, 1)) << std::endl;
        std::cout << compressedLoc.angle(Location(2, 1), Location(1, 2)) << std::endl;
        std::cout << compressedLoc.angle(Location(1, 1), Location(0, 0)) << std::endl;
        std::cout << compressedLoc.angle(Location(0, 0), Location(1, 0)) << std::endl;
        std::cout << compressedLoc.angle(Location(1, 0), Location(0, 0)) << std::endl;
        std::cout << compressedLoc.angle(Location(0, 1), Location(0, 0)) << std::endl;
        std::cout << compressedLoc.angle(Location(0, 0), Location(0, 1)) << std::endl;
        return 0;
    }

    double longitude, latitude;
    std::cin >> longitude >> latitude;
    Location ploc = Location(latitude, longitude);
    std::cout.setf(std::ios::fixed);
    int pan, an = 500;
    size_t count = 0;
    while (!std::cin.eof()) {
        std::cin >> longitude >> latitude;
        if (std::cin.fail()) break;
        count += 1;
        compressedLoc.push_back(Location(latitude, longitude));
        if (false) {
            Location here = Location(latitude, longitude); 
            if (compressedLoc.distance_between_points(here, ploc) > 20) {
                an = compressedLoc.angle(here, ploc); 
                std::cout << std::setw(3) << an << "\t" <<  compressedLoc.difangle(an, pan)  << std::endl;
                pan = an;
                ploc = here;
            }
        }
        /*std:: cout << std::setprecision(8)<< "(" << plongitude << ", " << platitude << " ---- " << longitude << ", " << latitude << "):";
        std:: cout << loc.distance_between_points(Location(platitude, plongitude), Location(latitude, longitude)) << "\t" << loc.distance_between_points2(Location(platitude, plongitude), Location(latitude, longitude)) << std::endl;*/
    }

    for (auto &l : compressedLoc.compressedData) {
        cout << l.longitude << "\t" << l.latitude << endl;
    }

    cout << count << "\t" << compressedLoc.compressedData.size() << endl;

    return 0;
}

int main(int , char *[]) {
    CompressedDouble cd = CompressedDouble(10);
	
    cd.push_back(1.12);
    cd.push_back(1.13);
    cd.push_back(1.14);
    cd.push_back(2.0);
    cd.push_back(3.0);
    cd.push_back(3.1);
    cd.push_back(3.13);
    cd.push_back(3.13213);

    CompressedInt ci = CompressedInt(10);
    ci.push_back(10);
    ci.push_back(11);
    ci.push_back(20);

    CompressedBool cb = CompressedBool();
    cb.push_back(true);
    cb.push_back(false);
    cb.push_back(true);
    cb.push_back(true);
}


