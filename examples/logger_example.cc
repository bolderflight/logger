/*
* Brian R Taylor
* brian.taylor@bolderflight.com
* 
* Copyright (c) 2020 Bolder Flight Systems
*/

#include "logger/logger.h"

/* SD object */
SdFat32 sd;
/* Datalog object, 200 FIFO depth */
Logger<200> datalog(&sd);
/* Counter to log */
volatile unsigned int counter = 0;
/* Data acquisition ISR */
void daq() {
  if (counter < 1000) {
    datalog.Write((uint8_t *)&counter, sizeof(counter));
    counter++;
  }
}

int main() {
  /* Start serial for communication */
  Serial.begin(115200);
  while(!Serial){}
  Serial.println("Begin test");
  /* Start SD */
  sd.begin(SdioConfig(FIFO_SDIO));
  /* Init datalog */
  int log_num = datalog.Init("test_data");
  if (log_num < 0) {
    Serial.println("FAILED TO INIT");
    while(1){}
  } 
  /* Create a timer to trigger the ISR */
  IntervalTimer daq_timer;
  daq_timer.begin(daq, 5000);
  /* Run the ISR 1000 times */
  while (counter < 1000) {
    /* Write data to SD as a low priority loop (the ISR will have higher priority) */
    datalog.Flush();
  }
  /* Close the data log after the test */
  datalog.Close();
  Serial.println("Done");
  while(1){}
}
