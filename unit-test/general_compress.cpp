/*
 * =====================================================================================
 *
 *       Filename:  general_compress.cpp
 *
 *    Description:  用来测试通用压缩
 *
 *        Version:  1.0
 *        Created:  05/26/2017 12:36:27 PM
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
#include "../cache-node/compress.h"
#include <vector>

using namespace std;

int main() {
    vector<char> v = {1, 1, 1, 1, 0, 0, 0, 1, 1 , 0};
    GeneralTimeline<char> timeline;
    time_t t = 0;
    for (char c : v) {
	timeline.push_back(c, t ++);
    }
    return 0;
}
