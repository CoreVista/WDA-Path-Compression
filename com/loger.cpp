//
// Created by lenfien on 16-10-21.
//
#include <iostream>
#include "loger.h"

pthread_t Loger::_logThread;
Queue<std::pair<std::string, Loger::LogType>> Loger::_msgQue;
bool Loger::_logStarted = false;
//std::ofstream Loger::_logofs("/var/log/buglog", std::ios_base::app);
std::ostream& Loger::_logofs = std::cout;
