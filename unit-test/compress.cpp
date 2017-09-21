#include <iostream>
#include <vector>
#include <chrono>
#include <array>
#include <string>
using namespace std;

int main(int argc, char *argv[]) {
    auto beg = chrono::high_resolution_clock::now();
    
    size_t count = strtoll(argv[1], nullptr, 10);
    array<vector<string>, 20> vecs;
    
    cout << "准备写入" << count << "条数据" << endl;
    for (size_t i = 0; i < count; ++i) {
        for (array<vector<string>, 20>::size_type i = 0; i < vecs.size(); ++i) {
            if (std::rand() % 100 < 10)
                vecs[i].push_back("a");
        }
    }
    
    auto end = chrono::high_resolution_clock::now();

    cout << "用时" << chrono::duration_cast<chrono::milliseconds>(end - beg).count() << "ms" << endl;
}
