/*
 * =====================================================================================
 *
 *       Filename:  json-test.cpp
 *
 *    Description:  `
 *
 *        Version:  1.0
 *        Created:  05/22/2017 12:48:24 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <string>

#include "../dep/rapidjson/writer.h"
#include "../dep/rapidjson/document.h"
#include "../dep/rapidjson/stringbuffer.h"

using namespace rapidjson;

int main(int, char *[]) {
    const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";
    rapidjson::Document d;
    d.Parse(json);
    auto& a = d["a"];
    a.PushBack(10, d.GetAllocator());
    for (auto it = a.Begin(); it != a.End(); ++it) {
	std::cout << it->GetInt() << std::endl;
    }
    Value author;
    author.SetString("helloworld", d.GetAllocator());

    d.AddMember("hu", author, d.GetAllocator());
    std::cout << d["hu"].GetString() << std::endl;
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    std::cout << buffer.GetString() << std::endl;
    return 0;
}
