/*
* Brian R Taylor
* brian.taylor@bolderflight.com
* 
* Copyright (c) 2021 Bolder Flight Systems Inc
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the “Software”), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#include "logger/logger.h"

/* SD object */
SdFat32 sd;
/* Datalog object, 200 FIFO depth */
bfs::Logger<200> datalog(&sd);
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
