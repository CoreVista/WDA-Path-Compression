/*
 * =====================================================================================
 *
 *       Filename:  file.h
 *
 *    Description:  
 *
 *
 *        Version:  1.0
 *        Created:  05/28/2017 10:07:01 PM
 *       Revision:  none
 *       Compiler:  gcc
 * *         Author:  张宏博 (), lenfien@foxmail.com *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __INCLUDE_FILE
#define __INCLUDE_FILE

#include <string>
#include <vector>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <atomic>
#include <fstream>
#include <set>

#include "../cache-node/data.h"
#include "../com/loger.h"

class CSVHelper {
private:
    std::string rootDir;
    std::vector<std::string>	titles;
    std::vector<std::string>	csvFiles;	    /* 需要被处理的csv文件 */
    std::vector<size_t>		indexOfFiles;
    std::vector<size_t>		indexOfRecords;
    std::vector<std::pair<size_t, size_t>> fromTo;
    std::vector<std::vector<std::vector<std::string>>> recordsPerFile;
    std::atomic_size_t		recordsCount;
    std::vector<std::ifstream>	currentIfs; /* 当前正在处理的文件 */

public:
    CSVHelper(const std::string& rootDir):rootDir(rootDir) { 
	recordsCount = 0;
	process_directory(); } 

    ~CSVHelper() {}

private:
    /* 建立目录索引 */
    void process_directory(std::string path);

    /* 建立目录索引 */
    void process_directory();

    /* 处理行 */
    std::vector<std::string> process_line(std::string& line);

    /* 下一文件 */
    bool next_file(size_t thr);

    /* 处理所有文件 */
    void process_files();

public:
    /* 将所有文件分成s个子集，方便多线程处理 */
    int  split(size_t s);

    /* 复原next操作 */
    void reset();

    /* 获取下一个记录 */
    bool next_record(size_t id, std::vector<std::string>& fields); /* 调用该函数可以卓条获取记录 */

    /* 将GB18030转换成UTF-8 */
    void iconv(const std::string& target);

    /* 汇报进度 */
    size_t progress(size_t thr);

    /* 提取indexOfFields */
    void extract(size_t id, const std::set<size_t>& indexOfFields, std::ofstream& os);

    /* 
     * 按照车辆编号整理记录
     * 不支持多线程处理, 所以在调用之前应该先split(1)
     */
    void arrange(std::string outDir);

    /* 排序, 输出文件 */ 
    void sort(size_t thr, const std::string& outdit);

    /* 获取已经通过next_record获取到的count */
    size_t record_count() { return recordsCount; }

    size_t csvfile_count() {return csvFiles.size(); }

    const std::string& filename(size_t n) {return csvFiles[n];}
};

inline void CSVHelper::reset() {
    for (size_t i = 0; i < fromTo.size(); ++i) {
	indexOfFiles[i] = fromTo[i].first;
	indexOfRecords[i] = 0;
    }
    recordsCount = 0;
}

inline size_t CSVHelper::progress(size_t thr) { 
    return (indexOfFiles[thr] - fromTo[thr].first) * 100 / (fromTo[thr].second - fromTo[thr].first);
}

#endif
