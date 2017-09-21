/*
     * =====================================================================================
     *
     *       Filename:  file.cpp
     *
     *    Description:  主要处理车联网数据文件
     *
     *        Version:  1.0
     *        Created:  05/28/2017 09:25:48 PM
     *       Revision:  none
     *       Compiler:  gcc
     *
     *         Author:  张宏博 (), lenfien@foxmail.com
     *   Organization:  
     *
     * =====================================================================================
 */
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <fcntl.h>
#include <stdio.h>
#include <algorithm>

#include "../com/loger.h"
#include "../com/extension.h"
#include "file.h"

using namespace std;

std::vector<std::string> CSVHelper::process_line(std::string& line) {
    using namespace std;
    vector<string> fields;
    std::string::size_type posb, pose;
    posb = 0;
    for (pose = line.find(",", 0); pose != line.npos; posb = pose + 1, pose = line.find(",", posb)) {
	size_t length = pose - posb;
	if (line[posb] == '\"') {
	    posb += 1;
	    length -= 1;
	}
	if (line[pose - 1] == '\"') length -= 1;
	fields.push_back (line.substr(posb, length));
    }
    fields.push_back(line.substr(posb));
    return fields;
}

void CSVHelper::process_directory() {
    Loger::log("正在建立目录索引");
    process_directory(rootDir);
    Loger::log("目录索引建立完成");
}

void CSVHelper::process_directory(std::string path) {
    struct dirent ent;
    struct dirent *res;
    struct stat sta;
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
	Loger::err("opendir" + path);
	return;
    }

    for (int err = readdir_r(dir, &ent, &res); err == 0 && res != nullptr; err = readdir_r(dir, &ent, &res)) {
	std::string fname = ent.d_name;
	if (fname == "." || fname == "..")
	    continue;

	std::string fpath = path + "/" + fname;
	err = stat(fpath.c_str(), &sta);
	if (err == -1) {
	    Loger::err(fpath);
	    return;
	}

	if (sta.st_mode & S_IFDIR) {
	    string npath = path + "/" + fname;
	    process_directory(npath);
	    continue;
	}

	if ((sta.st_mode & S_IFREG) && fname.rfind(".csv") != fname.npos) {
	    string npath = path + "/" + fname;
	    csvFiles.push_back(npath);
	}
    }
    closedir(dir);
}

bool CSVHelper::next_record(size_t thr, std::vector<std::string> &fields) {
    if (indexOfRecords[thr] >= recordsPerFile[thr].size()) {
	do { if (next_file(thr) == false) return false; }
	while (recordsPerFile[thr].size() == 0);
    }
    recordsCount += 1;
    fields = recordsPerFile[thr][indexOfRecords[thr]++];
    return true;
}

bool CSVHelper::next_file(size_t thr) {
    std::ifstream& ifs = currentIfs[thr];
    if (!ifs.is_open()) {
	if (indexOfFiles[thr] >= fromTo[thr].second)
	    return false;

	string path = csvFiles[indexOfFiles[thr]++];
	ifs.open(path);
	if (!ifs) {
	    Loger::err("打开文件失败[" + path + "]");
	    return false;
	}
    }

    std::string header, line;
    getline(ifs, header);
    if (titles.empty())
	titles = process_line(header);

    size_t maxLines = 1024;
    recordsPerFile[thr].clear();
    while (maxLines-- && ifs && getline(ifs, line)) {
	if (line.empty()) continue;
	vector<string> fields = process_line(line);
	recordsPerFile[thr].push_back(fields);
    }

    if (!ifs)
	ifs.close();

    indexOfRecords[thr] = 0;
    return true;
}

void CSVHelper::process_files() {
    std::string header;
    std::string line;

    for (auto path: csvFiles) {
	ifstream is(path);
	if (!is) {
	    Loger::err(path);
	    return;
	}

	getline(is, header);
	if (titles.size() == 0)
	    titles = process_line(line);

	Loger::log (header);
	while (getline(is, line)) {
	    vector<string> fields = process_line(line);
	    for (auto f : fields) {
		Loger::log(f);
	    }
	}
    }
}

void CSVHelper::iconv(const std::string& target) {
    int fname = 0;
    mkdir(target.c_str(), 0776);
    for (auto& file: csvFiles) {
	std::string command = "iconv -f GB18030 -t UTF-8 '" + file + "' -o " + target  + "/" + std::to_string(fname++) + ".csv";
	int err = system(command.c_str());
	if (err != 0) {
	    Loger::err("system excute fail");
	}
    }
}

int CSVHelper::split(size_t s) {
    size_t from, to = 0;
    size_t st = csvFiles.size() / s;
    if (st * s < csvFiles.size()) st += 1;

    if (s > csvFiles.size()) {
	Loger::fatal("split的个数大于csvfile的个数");
	return 0;
    }

    for (size_t i = 0; i < s; ++i) {
	from = to;
	to = from + st;
	if (to > csvFiles.size()) to = csvFiles.size();
	fromTo.push_back(std::make_pair(from, to));
	if (to == csvFiles.size())
	    break;
    }

    recordsPerFile.assign(fromTo.size(), {});
    indexOfFiles.assign(fromTo.size(), 0);
    indexOfRecords.assign(fromTo.size(), 0);
    for (size_t i = 0; i < fromTo.size(); ++i) {
	indexOfFiles[i] = fromTo[i].first;
    }
    for_each(currentIfs.begin(), currentIfs.end(), [](std::ifstream& ifs) { ifs.close(); });
    currentIfs.resize(fromTo.size());
    recordsCount = 0;
    return fromTo.size();
}

void CSVHelper::extract(size_t id, const std::set<size_t>& indexOfFields, ofstream& os) {
    std::vector<std::string> fields;
    std::string line;
    line.reserve(128);
    bool header = false;
    size_t prog = 0;
    while (next_record(id, fields)) {
	recordsCount += 1;
	if (!header) {
	    os << titles;
	    header = true;
	}

	if (fields.size() == 0)
	    Loger::log("Empty fields");

	bool success = true;
	for (size_t i = 0; i < fields.size(); ++i) {
	    if (indexOfFields.find(i + 1) != indexOfFields.end()) {
		if (i != 2 && i != 3 && i != 4 && i != 5) {
		    if (fields[i].empty() || fields[i] == "无效" || fields[i] == "OFF" || fields[i] == "0") {
			line += "0";
		    }
		    else {
			line += "1";
		    }
		}
		else {
		    if (i == 3) {
			struct tm _tm;
			success = true;
			memset(&_tm, 0, sizeof(_tm));
			int n = sscanf (fields[i].c_str(), "%d-%d-%d %d:%d:%d", &_tm.tm_year, &_tm.tm_mon, &_tm.tm_mday, &_tm.tm_hour, &_tm.tm_min, &_tm.tm_sec);
			if (n != 6) {
			    memset(&_tm, 0, sizeof(_tm));
			    n = sscanf (fields[i].c_str(), "%d/%d/%d %d:%d:%d", &_tm.tm_year, &_tm.tm_mon, &_tm.tm_mday, &_tm.tm_hour, &_tm.tm_min, &_tm.tm_sec);
			    if (n != 6) {
				memset(&_tm, 0, sizeof(_tm));
				n = sscanf (fields[i].c_str(), "%d/%d/%d %d:%d", &_tm.tm_year, &_tm.tm_mon, &_tm.tm_mday, &_tm.tm_hour, &_tm.tm_min);
				if (n != 5) {
				    success = false;
				}
			    }
			}

			if (success) {
			    _tm.tm_mon -= 1;
			    _tm.tm_year -= 1900;
			    time_t t = mktime(&_tm);
			    line += std::to_string(t);
			}
		    }
		    else
			line += fields[i];
		}
		line += ",";
	    }
	}
	
	if (!success) { Loger::log("跳过"); }
	if (!line.empty() && success) {
	    line.pop_back();
	    os << line << std::endl;
	    line.clear();
	}

	if (progress(id) != prog) {
	    prog = progress(id);
	    Loger::log ("thread " + std::to_string(id) + ":" + std::to_string(prog));
	}
    }
    os.flush();
}

void CSVHelper::arrange(std::string outDir) {
    using namespace std;
    std::map<string, ofstream*> outFile;
    vector<string> fields;
    size_t progp = 0;
    size_t prog = 0;
    mkdir(outDir.c_str(), 0776);
    errno = 0;

    while (next_record(0, fields)) {
	if (fields.size() == 0) 
	    continue;

	const string& no = fields[0];
	if (no[0] != 'W') 
	    continue;

	ofstream* os = outFile[no];
	if (os == nullptr) {
	    string outpath = outDir + "/" + no + + ".csv";
	    os = new ofstream(outpath, std::ios_base::app);
	    if (!*os) {
		Loger::warning("打开" + outpath);
		continue;
	    }
	    outFile[no] = os;
	    *os << titles;
	}

	*os << fields;
	recordsCount += 1;
	prog++;

	if (prog - progp >= 100000) {
	    progp = prog;
	    Loger::log ("进度" + std::to_string(prog));
	}
    }

    for (auto& v : outFile) {
	if (v.second) {
	    v.second->close();
	    delete v.second;
	}
    }
}

void CSVHelper::sort(size_t thr, const std::string& outdir) {
    std::vector<std::vector<std::string>> records;
    std::vector<std::string> fields; 
    std::string outname;
    while (next_record(thr, fields)) {
	if (outname.empty())
	    outname = outdir + "/" + fields[0] + ".csv";
	if (fields.size() == 24) {
	    records.push_back(fields);
	}
	else {
	    Loger::log("jump");
	}
    }

    std::sort(records.begin(), records.end(), 
	    [](std::vector<std::string>& l, std::vector<std::string>& r) {
		return l[1] < r[1];
	    });

    std::ofstream ofs(outname, std::ios_base::trunc);
    if (!ofs)  {
	Loger::err(outname);
	return;
    }
    ofs << titles;
    for (auto& re: records) {
	ofs << re;
    }
    ofs.close();
}
