// Universidad del Valle de Guatemala
// CC3056
// Christian Medina

// compile with:
// g++ main.cpp bme280.cpp bme280.h -lwiringPi -o bme280

#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <wiringPiI2C.h>
#include "bme280.h"
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <iostream>
#include <ctime>
#include <string.h>

#define TIMESLEEP 3000000
using namespace std;
ofstream myfile;

int main(int argv, char *argc[])
{
  // Boolean isRunning = true
  char key; // Para detectar la tecla presionada
  bool firstRun = true;
  // Max, Min y Average
  int max = 0;
  int min;
  float average;
  int totalSum = 0;
  // para guardar fechas
  time_t now = time(0);
  string thisDate;
  string thisTime;
  tm *ltm = localtime(&now);

  // Setup del chip
  int fd = wiringPiI2CSetup(BME280_ADDRESS);
  
  if (fd < 0)
  {
    printf("Device not found");
    return -1;
  }
  // Crear archivo y titulos de columnas
  myfile.open("template.csv");
  myfile << "Temperatura, Humedad, Presion, Fecha, Hora\n";

  bme280_calib_data cal;
  readCalibrationData(fd, &cal);

  wiringPiI2CWriteReg8(fd, 0xf2, 0x01); // humidity oversampling x 1
  wiringPiI2CWriteReg8(fd, 0xf4, 0x25); // pressure and temperature oversampling x 1, mode normal

  bme280_raw_data raw;
  int counter = 0;
  while (true)
  {
    if (kbhit())
    {
      key = _getch();
      if (isdigit(key))
      {
        break;
      }
    }
    else
    {
      counter = counter + 1;
      getRawData(fd, &raw);

      int32_t t_fine = getTemperatureCalibration(&cal, raw.temperature);

      float t = compensateTemperature(t_fine);                        // C
      float p = compensatePressure(raw.pressure, &cal, t_fine) / 100; // hPa
      float h = compensateHumidity(raw.humidity, &cal, t_fine);       // %
      float a = getAltitude(p); 
                                            // meters
      tm *ltm = localtime(&now);
      thisDate = "" + to_string(ltm->tm_mday) + "/" + to_string(ltm->tm_mon) + "/" + to_string((ltm->tm_year) % 100);
      thisTime = "" + to_string(1+ltm->tm_hour) + ":" +to_string(1 + ltm->tm_min) + ":" + to_string(1 + ltm->tm_sec);

      if (firstRun == true)
      {
        min = t;
      }
      if (max < t)
      {
        max = t;
      }
      if (min > t && firstRun == false)
      {
        min = t;
      }
      totalSum = totalSum + t;
      myfile << t << "," << h << "," << p << thisDate << "," << thisTime << "," << endl;

      usleep(TIMESLEEP); // 3 seg
    }
  }
  average = (float)totalSum / counter;
  printf("Max: %d\nMin: %d, Average: %f\n", max, min, average);
  myfile.close();
  exit(EXIT_SUCCESS);
}