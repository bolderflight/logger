/*
* Brian R Taylor
* brian.taylor@bolderflight.com
* 
* Copyright (c) 2020 Bolder Flight Systems
*/

#include "logger/logger.h"

Logger<200> datalog;

unsigned int counter = 0;

void daq() {
  datalog.Write((uint8_t *)&counter, sizeof(counter));
  counter++;
  // Serial.println(counter);
}

int main() {
  Serial.begin(115200);
  while(!Serial){}
  Serial.println("Begin test v2");
  if (!datalog.Init("test_data")) {
    Serial.println("FAILED TO INIT");
  } else {
    Serial.println("GOOD INIT");
  }
  IntervalTimer daq_timer;
  daq_timer.begin(daq, 5000);
  while (1) {
    datalog.Flush();
  }
}
