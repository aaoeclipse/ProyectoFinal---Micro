#include <iostream>
#include <ctime>
#include <string.h>

using namespace std;
using std::string;

void getDateAndTime();
int main()
{
    getDateAndTime();
}

void getDateAndTime()
{
    // current date/time based on current system
    time_t now = time(0);
    
    tm *ltm = localtime(&now);
    // string thisDate = "" + to_string(ltm->tm_mday) + "/" + to_string(ltm->tm_mon) + "/" + to_string((ltm->tm_year) % 100) + "\n";
    // cout << thisDate;
    string thisTime = "" + to_string(1+ltm->tm_hour) + ":" +to_string(1 + ltm->tm_min) + ":" + to_string(1 + ltm->tm_sec) + "\n";
    // cout << "Time: "<< 1 + ltm->tm_hour << ":";
    cout << thisTime;
    //    cout << 1 + ltm->tm_min << ":";
    //    cout << 1 + ltm->tm_sec << endl;
}