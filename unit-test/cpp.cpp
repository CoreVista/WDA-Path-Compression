/*
 * =====================================================================================
 *
 *       Filename:  cpp_testing.cpp
 *
 *    Description: l
 *
 *        Version:  1.0
 *
 *        Created:  05/27/2017 12:59:58 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <vector>
#include <fstream>
#include "../cache-node/data.h"

using namespace std;

int main(int , char *[]) {
    ifstream ifs;
    string a;
    std::cout << sizeof(double) << std::endl;
    vector<int> v;
    v.reserve(1000);
    v.push_back(1);
    cout << v.capacity() << endl;
    cout << v.size() << endl;
    return 0;
}
