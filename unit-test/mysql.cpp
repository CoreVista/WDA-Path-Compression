#include <cppconn/driver.h>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <string>
#include <chrono>
#include <iostream>

using namespace std;

sql::Driver* driver;
sql::Connection* con;
sql::Statement *state;
int main(int argc, char *argv[]) {
    int count = 20000;
    if (argc == 2) {
        count = strtol(argv[1], nullptr, 10);
    }

    cout << "准备写入" << count << "条记录" << endl;

    clock_t beg = clock();
    auto nbeg = chrono::high_resolution_clock::now();

    driver = get_driver_instance();
    if (driver == nullptr) {
        cerr << "get driver failed" << endl;
        return 1;
    }

    try {
        con = driver->connect("localhost", "root", "3646");
        state = con->createStatement();
	
        state->execute("USE testDatabase");
        state->execute("DELETE FROM car");

        for (int i = 0; i < count; ++i) {
            state->execute("INSERT INTO car VALUE('1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17', '18', '19', '20')");
        }
        con->close();
    }
    catch (sql::SQLException& ex) {
        cout << ex.what() << endl;
    }

    clock_t end = clock();

    auto nend = chrono::high_resolution_clock::now();
    cout << ((end - beg) * 1000 / CLOCKS_PER_SEC) << "ms" << endl;
    cout << chrono::duration_cast<chrono::milliseconds>(nend - nbeg).count() << "ms" << endl; 
    return 0;
}
