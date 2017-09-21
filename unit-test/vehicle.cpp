
#include <string.h>
#include "../cache-node/cache.h"
#include "../cache-node/data.h"

int main(int , char *[]) {
    Cache cache;
    VehicleData data;
    data.id = "G1234";
    data.longitude = 12.1;
    data.latitude = 12.3;
    
    std::vector<unsigned char> pack = make_package(1, data);
    Message msg(0, 0, std::move(pack));
    cache.push_back(msg);
    cache.print();
    return 0;

    VehicleData vdata(msg);
    Package ppack = make_package(1, vdata);
    Package encodedPack = encode_package(ppack);
    Package decodedPack = decode_package(encodedPack);
    return 0;
}
