#ifndef __INCLUDE_EPOLL
#define __INCLUDE_EPOLL

#include <sys/epoll.h>
#include <memory>
#include <pthread.h>
#include <atomic>
#include <thread>
#include "../com/queue.h"
#include "../com/loger.h"

#include "data.h"

class Epoll {

private:
    struct Connection {
	uint64_t    sid;
	int	    sock;
	PackageMonitor	monitor;
	std::string	clientName;

	Connection():sid(0), sock(-1) {}
	Connection(uint64_t sid, int sock): sid(sid), sock(sock) {}

	void clear() {
	    sock = -1;
	    sid = 0;
	    monitor.reset();
	    clientName.clear();
	}
    };

private:
    int epollFD;
    int bsock;
    pthread_t epollThread;

    Queue<std::shared_ptr<Message>> out;
    Queue<std::shared_ptr<Message>> in;

    std::array<struct epoll_event, 1024> epEvents;
    std::vector<Connection> sock2Connection;
    std::array<unsigned char, 1024> buffer;
    static std::atomic<uint64_t> sessionId;				    /* 所有的ConnectionId都是在该数基础上+1 */
    uint16_t port = 0;

public:
    Epoll(const std::string& port) {
	int err = pthread_create(&epollThread, nullptr, epoll_thread, this);
	if (err != 0) {
	    Loger::err("pthread_create fail");
	    return;
	}
	this->port = std::stoi(port);
    }

public:
    inline std::shared_ptr<Message> pop_message() {
	std::shared_ptr<Message> msg;
	out.get(msg);
	return msg;
    }

    inline std::shared_ptr<Message> try_pop() {
	std::shared_ptr<Message> msg;
	if (out.try_pop(msg))
	    return msg;
	return nullptr;
    }

    inline void push_message(std::shared_ptr<Message> msg) {
	in.put(msg);
    }

    Queue<std::shared_ptr<Message>>& get_outQueue() {return out;}
    Queue<std::shared_ptr<Message>>& get_inQueue() {return in;}

/************************************************************/
public:
    static void* epoll_thread(void *param);
};

#endif
