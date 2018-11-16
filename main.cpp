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
#include <cstdio>
#include <sys/select.h>
#define TIMESLEEP 10
using namespace std;
ofstream myfile;

int kbhit(void)
{
  struct timeval tv;
  fd_set read_fd;

  /* Do not wait at all, not even a microsecond */
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  /* Must be done first to initialize read_fd */
  FD_ZERO(&read_fd);

  /* Makes select() ask if input is ready:
   * 0 is the file descriptor for stdin    */
  FD_SET(0, &read_fd);

  /* The first parameter is the number of the
   * largest file descriptor to check + 1. */
  if (select(1, &read_fd, NULL, /*No writes*/ NULL, /*No exceptions*/ &tv) == -1)
    return 0; /* An error occured */

  /*  read_fd now holds a bit map of files that are
   * readable. We test the entry for the standard
   * input (file 0). */

  if (FD_ISSET(0, &read_fd))
    /* Character pending on stdin */
    return 1;

  /* no characters were pending */
  return 0;
}

/**
* para evitar tener que usar usleep que puede fallar con enteros grandes...
*/
void wait(int seg)
{
  clock_t endwait;
  endwait = clock() + seg * CLOCKS_PER_SEC;
  while (clock() < endwait)
  {
    //no hace nada ...
  }
}
/**
* retorna maximo y minimo en un array, maximo en posicion 0 y minimo en 1.
*/

float *maxMinFunction(bool first, float t, float *minmax)
{
  float *results = new float[2];
  results[0] = roundf(minmax[0] * 100) / 100;
  results[1] = roundf(minmax[1] * 100) / 100;
  t = roundf(t * 100) / 100;

  if (first == true)
  {
    cout << "newMax y newMin" << endl;
    results[0] = t;
    results[1] = t;
  }
  else
  {
    if (results[0] < t) // Max es menor al nuevo temp
    {
      cout << "newMax: " << t << " > " << results[0] << endl;
      results[0] = t;
    }
    if (results[1] > t) // Min is menor al nuevo temp
    {
      cout << "newMin: " << results[1] << " > " << t << endl;
      results[1] = t;
    }
  }
  return results;
}

int main(int argv, char *argc[])
{
  cout << "Este programa utiliza el BME280, tomara datos cada 3 seg.\nPara parar la ejecucion presione una tecla numerica y un enter." << endl;
  // Boolean isRunning = true
  char key; // Para detectar la tecla presionada
  bool firstRun = true;
  bool changeDay = false;
  // Max, Min y Average
  float max = 0;
  float min;
  float average;
  //se almacena en un vector el resultado de funcion maxMin..
  float *maxMin = new float[2];
  maxMin[0] = 0;
  maxMin[1] = 0;
  float totalSum = 0;
  float t; // C
  float p; // hPa
  float h; // %
  float a;
  // para guardar fechas
  time_t now = time(0);
  string thisDate;
  string thisTime;
  string fileName;
  tm *ltm = localtime(&now);

  // Setup del chip
  int fd = wiringPiI2CSetup(BME280_ADDRESS);

  if (fd < 0)
  {
    printf("Device not found");
    return -1;
  }

  //obtenemos valoress de fecha y hora...
  fileName = "" + to_string(ltm->tm_year) + to_string(ltm->tm_mon) + to_string(ltm->tm_mday);

  // Crear archivo y titulos de columnas
  myfile.open(fileName + ".csv");
  myfile << "Temperatura, Humedad, Presion, Fecha, Hora\n";

  bme280_calib_data cal;
  readCalibrationData(fd, &cal);

  wiringPiI2CWriteReg8(fd, 0xf2, 0x01); // humidity oversampling x 1
  wiringPiI2CWriteReg8(fd, 0xf4, 0x25); // pressure and temperature oversampling x 1, mode normal

  bme280_raw_data raw;
  // estaba en el while
  getRawData(fd, &raw);
  int32_t t_fine = getTemperatureCalibration(&cal, raw.temperature);

  int counter = 0;
  while (true)
  {
    if (kbhit())
    {
      key = getchar();
      if (isdigit(key))
      {
        break;
      }
    }
    else
    {
      fd = wiringPiI2CSetup(BME280_ADDRESS);
      
      readCalibrationData(fd, &cal);

      wiringPiI2CWriteReg8(fd, 0xf2, 0x01); // humidity oversampling x 1
      wiringPiI2CWriteReg8(fd, 0xf4, 0x25); // pressure and temperature oversampling x 1, mode normal
      // readCalibrationData(fd, &cal);
      getRawData(fd, &raw);
      t_fine = getTemperatureCalibration(&cal, raw.temperature);

      counter = counter + 1;
      // getRawData(fd, &raw);

      t = compensateTemperature(t_fine);                        // C
      p = compensatePressure(raw.pressure, &cal, t_fine) / 100; // hPa
      h = compensateHumidity(raw.humidity, &cal, t_fine);       // %
      a = getAltitude(p);
      // meters
      now = time(0);
      tm *ltm = localtime(&now);

      thisDate = "" + to_string(ltm->tm_mday) + "/" + to_string(ltm->tm_mon) + "/" + to_string((ltm->tm_year) % 100);
      thisTime = "" + to_string(ltm->tm_hour) + ":" + to_string(ltm->tm_min) + ":" + to_string(ltm->tm_sec);

      //si llega a 00 horas, inicia de nuevo jaja... y crea archivo nuevo
      // ltm->tm_hour | "00"
      if (to_string(ltm->tm_hour).compare("00") == 0 && changeDay == false)
      {
        myfile.close();
        // ltm->tm_mday 
        myfile.open(to_string(ltm->tm_mday) + to_string(ltm->tm_mon) + to_string((ltm->tm_year)) + ".csv");
        myfile << "Temperatura, Humedad, Presion, Fecha, Hora\n";
        changeDay = true;
      }

      //retorna maximo y minimo en un array, maximo en posicion 0 y minimo en 1.
      maxMin = maxMinFunction(firstRun, t, maxMin);

      firstRun = false;

      totalSum = totalSum + t;
      //escritura en archivo...

      myfile << t << "," << h << "," << p << "," << thisDate << "," << thisTime << endl;
      cout << t << "\t" << h << "\t" << p << "\t" << thisDate << "\t" << thisTime << '\t' << counter << endl;
      wait(TIMESLEEP); // 3 seg
    }
  }
  average = (float)totalSum / counter;
  printf("Max: %f\nMin: %f, Average: %f\n", maxMin[0], maxMin[1], average);
  myfile.close();
  exit(EXIT_SUCCESS);
}