
cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
project(cache-sys)

add_compile_options(-std=c++11)
add_compile_options(-g)
add_compile_options(-O3)
add_compile_options(-Wall)
add_compile_options(-I/usr/include)
add_compile_options(-I/usr/include/x86_64-linux-gnu)
add_compile_options(-I/usr/include/c++/5.4.0)

set(TESTING_SOURCES ./LocationCompressTesting.cpp)

set(COMMON_SOURCE 
    ./com/locker.h 
    ./com/loger.cpp
    ./com/loger.h
    ./com/queue.h
    ./com/system.h
    ./com/extension.h)

set(CACHE_NODE_SOURCES 
    ./cache-node/data.h
    ./cache-node/data.cpp
    ./cache-node/cache.h
    ./cache-node/cache.cpp
    ./cache-node/epoll.cpp
    ./cache-node/epoll.h
    ./cache-node/worker.h)

set(TERMINAL_SOURCES 
    ./terminal/file.h
    ./terminal/file.cpp)

link_libraries(pthread mysqlcppconn)

add_executable(cache-node		./cache-node/main.cpp		${CACHE_NODE_SOURCES} ${COMMON_SOURCE})
add_executable(test_client		./unit-test/client.cpp		${CACHE_NODE_SOURCES} ${COMMON_SOURCE})
add_executable(test_cpp			./unit-test/cpp.cpp		${CACHE_NODE_SOURCES} ${COMMON_SOURCE})
add_executable(test_dir			./unit-test/dir.cpp		${TERMINAL_SOURCES} ${COMMON_SOURCE})
add_executable(test_extra		./unit-test/extra.cpp		${TERMINAL_SOURCES} ${COMMON_SOURCE})
add_executable(test_arrange		./unit-test/arrange.cpp		${TERMINAL_SOURCES} ${COMMON_SOURCE}) 
add_executable(test_sort		./unit-test/sort.cpp		${TERMINAL_SOURCES} ${COMMON_SOURCE}) 
add_executable(terminal			./terminal/main.cpp		${TERMINAL_SOURCES} ${COMMON_SOURCE} ${CACHE_NODE_SOURCES})
add_executable(location 		./unit-test/location1.cpp 	${TERMINAL_SOURCES} ${COMMON_SOURCE})
