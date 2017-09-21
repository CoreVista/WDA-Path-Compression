/*
 * =====================================================================================
 *
 *       Filename:  location_compress.cpp
 *
 *    Description: 
 *
 *        Version:  1.0
 *        Created:  05/26/2017 01:34:29 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/time.h>
#include "../cache-node/compress.h"
#include  "time.h"

using namespace std;
int main(int , char *argv[]) {
    ifstream ifs (argv[1]);
    time_t t = 0;
    LocationTimeline line;
    Location loc;
    clock_t start;
    double compr_time, decompr_time;

    size_t sum = 0, n = 0;
    while (ifs >> loc.longitude && ifs >> loc.latitude) {
	line.push_back(loc, t++);
	sum += 1;
    }
    
    struct timeval beg, end;
    
    //gettimeofday(&beg, 0);
    start = clock();
    std::cout.setf(std::ios::fixed);
    auto clocs = line.timeline;
    for (auto& v : clocs) {
	for (auto& vv: v) {
	    std::cout << vv.st << " : " << vv.sv.longitude << "\t" << vv.sv.latitude << std::endl;
	    std::cout << vv.et << " : " <<  vv.ev.longitude << "\t" << vv.ev.latitude << std::endl;
	    n+=2;
	}
    }
    
     compr_time = (double)(clock() - start) / CLOCKS_PER_SEC;

    //gettimeofday(&end, 0);
    std::cout << n << "/" << sum << std::endl;
    
    double begms = beg.tv_sec * 1000 + beg.tv_usec/1000;
    double endms = end.tv_sec * 1000 + end.tv_usec/1000;

    std::cout << "用时:" <<  compr_time << "ms" << std::endl;
    


}
