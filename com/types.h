/*
 * =====================================================================================
 *
 *       Filename:  types.h
 *
 *    Description:  定义一些公用的类型
 *
 *        Version:  1.0
 *        Created:  05/21/2017 03:07:02 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __COM_TYPES_INCLUDE
#define __COM_TYPES_INCLUDE
#include <string>

struct Node {
    std::string host;
    std::string service;
};

#endif 
