#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <cstring>
#include <array>
#include <signal.h>
#include <arpa/inet.h>

#include "../com/loger.h"
#include "../com/extension.h"
#include "epoll.h"

using namespace std;
std::atomic<uint64_t> Epoll::sessionId(1);

void* Epoll::epoll_thread(void* param) {
    Epoll* pself = reinterpret_cast<Epoll*>(param);

    int err = 0;
    int epfd = epoll_create1(0);
    int bsock = socket (AF_INET, SOCK_STREAM, 0);

    auto& events = pself->epEvents;
    auto& baseSid = pself->sessionId;
    auto& sock2Connection = pself->sock2Connection;
    auto& buffer = pself->buffer;

    pself->epollFD = epfd;
    pself->bsock = bsock;

    Loger::log("Epoll system started");

    {
	int v;
	socklen_t l = sizeof(v);
	setsockopt(bsock, SOL_SOCKET, SO_REUSEADDR, &v, l);
	int flg = fcntl(bsock, F_GETFL);
	flg |= O_NONBLOCK;
	fcntl(bsock, F_SETFL, flg);
    }

    {
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pself->port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	err = bind(bsock, (struct sockaddr*)&addr, sizeof(addr));
	if (err == -1) {
	    Loger::err("Bind to port 8080 fail");
	    return nullptr;
	}

	err = listen(bsock, 100);
	if (err == -1) {
	    Loger::err("port fail");
	    return nullptr;
	}
    }

    Loger::log("EPOLL wait on " + std::to_string(pself->port));
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.fd = bsock;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, bsock, &event);

    for (;;) {
	int r = epoll_wait(epfd, events.data(), events.size(), 5000);

	if (r == -1) {
	    Loger::err("epoll_wait failed");
	    break;
	}

	if (r == 0) {
	    // TODO  在此加入周期性判断的事务
//	    Loger::log("epoll timout");
	    continue;
	}

	for (int n = 0; n < r; ++n) {
	    struct epoll_event& ev = events[n];
	    int sock = ev.data.fd;
	    if (ev.events & EPOLLIN) {
		if (sock == bsock) {        /* 处理新连接 */
		    struct sockaddr_storage saddr;
		    socklen_t l = sizeof(saddr);
		    int csock = 0;
		    while ((csock = ::accept(bsock, (struct sockaddr*)&saddr, &l)) > 0) {
			string clientName;
			switch (saddr.ss_family) {
			case AF_INET:
			    {
				char buf[INET_ADDRSTRLEN];
				sockaddr_in *sa = reinterpret_cast<sockaddr_in*>(&saddr);
				if (nullptr != ::inet_ntop(sa->sin_family, &sa->sin_addr, buf, sizeof(buf))) {
				    clientName = string(buf) + ":" + to_string(ntohs(sa->sin_port));
				}
			    }
			    break;

			default:
			    Loger::warning("没有处理的地址");
			    break;
			}

			uint64_t sid = ++baseSid;
			clientName += "-" + to_string(sid) + "-" + to_string(csock);
			clientName = "[" + clientName + "]";

			if (sock2Connection.size() <= static_cast<size_t>(csock))
			    sock2Connection.resize(csock + 1);

			Connection& connection = sock2Connection[csock];
			if (connection.sid != 0) Loger::warning("正在覆盖一个正在使用的Connection!");
			connection.clientName = clientName;
			connection.sid = sid;
			connection.sock = csock;

			epoll_event ev;
			ev.data.fd = csock;
			ev.events = EPOLLIN | EPOLLHUP;
			epoll_ctl(epfd, EPOLL_CTL_ADD, csock, &ev);
			Loger::log (clientName + "连接");
		    }

		    if (csock == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
			Loger::err("accept 返回-1但错误代码不是EAGAIN");
		    }
		    else {
			errno = 0;
		    }
		}
		else {
		    Connection& connection = sock2Connection[sock];
		    PackageMonitor& monitor = connection.monitor;
		    list<vector<unsigned char>> lists;
		    ssize_t rn = 0;

		    while ((rn = ::recv(sock, buffer.data(), buffer.size(), MSG_DONTWAIT)) > 0) {
			auto pn = monitor.put(buffer.data(), rn, lists);

			if (pn == -1) {
			    ::epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr);
			    ::close(sock);
			    Loger::warning(connection.clientName + "发来的数据解析出错");
			    connection.clear();
			    break;
			}
			else if (pn > 0) {
			    for (auto pl = lists.begin(); pl != lists.end(); ++pl) {
				pself->out.put(std::make_shared<Message>(connection.sock, connection.sid, std::move(*pl)));
			    }
			    lists.clear();
			}
		    }

		    if (rn == 0) {
			::epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr);
			::close(sock);
			Loger::log(connection.clientName + "断开");
			connection.clear();
		    }

		    if (rn == -1) {
			if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
			    ::epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr);
			    ::close(sock);
			    Loger::err(connection.clientName + "recv返回-1时errno不是EAGAIN");
			    connection.clear();
			}
			else {
			    errno = 0;
			}
		    }
		}
	    }
	    else if(event.events & EPOLLHUP) {
		Connection& connection = sock2Connection[sock];
		::epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr);
		::close(sock);
		Loger::log(connection.clientName + "断开连接");
		connection.clear();
	    }
	    else {
		Loger::warning(std::string("未处理的事件") + to_string(event.events));
	    }
	}
    }

    return 0;
}
