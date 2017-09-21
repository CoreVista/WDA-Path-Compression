#include <unistd.h>

#include <iostream>
#include "worker.h"
#include "epoll.h"
#include "compress.h"
#include "../com/loger.h"
using namespace std;

int main(int , char *argv[]) {
    Loger::log("Init OK");
    std::shared_ptr<Cache> cache = std::make_shared<Cache>();
    std::shared_ptr<Epoll> epoll = std::make_shared<Epoll>(argv[1]);
    std::shared_ptr<WorkerPool> worker = std::make_shared<WorkerPool>(cache, epoll->get_outQueue(), epoll->get_inQueue());

    while (true) pause();
    return 0;
}
