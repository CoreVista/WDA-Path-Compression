/*
 * =====================================================================================
 *
 *       Filename:  client.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/20/2017 05:59:03 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  张宏博 (), lenfien@foxmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <cstring>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#include "../cache-node/data.h"
#include "../com/loger.h"

using namespace std;

int main(int, char *[]) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    struct addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = 0;

    struct addrinfo* addrResult = nullptr;
    int err = getaddrinfo("localhost", "8080", &hint, &addrResult);
    if (err != 0) {
	Loger::err(gai_strerror(err));
	::exit(1);
    }

    struct sockaddr* add = addrResult[0].ai_addr;
    socklen_t len = addrResult[0].ai_addrlen;

    while (1) {
	vector<int> socks;
	for (int i = 0; i < 1; ++i) {
	    int sock = socket(AF_INET, SOCK_STREAM, 0);
	    err = ::connect(sock, add, len);
	    if (err == -1) {
		Loger::err("connect fail");
		exit(1);
	    }
	    socks.push_back(sock);
	    auto pack = make_package({1, 2, 3, 4, 5});
	    for (int i = 0; i < 1000; ++i) {
		::send(sock, pack.data(), pack.size(), 0);
	    }
	}
	for (int sock : socks) { ::close(sock); }
    }
}
