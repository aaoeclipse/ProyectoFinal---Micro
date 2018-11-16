#include <iostream> // std::cout
#include <string>   // std::string, std::stof
#include <fstream>

using namespace std;
void linearPrediction();
float getSlopeAverage(float *temp, int counter);
// struct tempStruct

int main()
{
    linearPrediction();
}

void linearPrediction()
{
    ifstream ip("CSVS/1181014.csv");

    if (!ip.is_open())
        std::cout << "ERROR: File Open" << '\n';

    string tempString;
    string::size_type sz;
    float *temp = new float(8640);
    string lastname;
    //   string age;
    //   string weight;
    int counter = 0;
    while (ip.good())
    {

        getline(ip, tempString, ',');
        temp[counter] = strtof((tempString).c_str(), 0);
        if (temp[counter] != 0)
        {
            printf("temp: %f\n", temp[counter]);
            counter = counter + 1;
            cout << "counter: " << counter << endl;
        }
        // next Line
        getline(ip, lastname, '\n');
        
    }
    printf("total: %d\n", counter);
    float average = getSlopeAverage(temp, counter);

    ip.close();

    fflush(stdout);

    cout << "average: " << average << endl;
    
    exit(EXIT_SUCCESS);
}

float getSlopeAverage(float *temp, int counter)
{
    float *total = new float(8640);
    float average;
    int countUp = 0;
    while ((counter-1) > countUp)
    {
        // cout << temp[countUp +1] << " - " << temp[countUp] << endl;  //debug
        total[countUp] = (temp[countUp + 1] - temp[countUp]);
        // cout << total[countUp] << endl;  //debug

        countUp = countUp + 1;
    }

    countUp = 0;
    average = 0;
    while ((counter-1) > countUp)
    {
        average = average + total[countUp];
        countUp = countUp + 1;
        // cout << "getting average" << average << endl;
    }
    average = average / (counter - 1);
    return average;
}