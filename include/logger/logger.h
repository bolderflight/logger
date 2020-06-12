/*
* Brian R Taylor
* brian.taylor@bolderflight.com
* 
* Copyright (c) 2020 Bolder Flight Systems
*/

#ifndef INCLUDE_LOGGER_LOGGER_H_
#define INCLUDE_LOGGER_LOGGER_H_

#include <string>
#include <atomic>
#include <limits>
#include "sd/SdFat.h"
#include "circle_buf/circle_buf.h"

template<unsigned int FIFO_DEPTH>
class Logger {
 public:
  explicit Logger(class SdFatSdioEX *sd_ptr) {
    sd_ptr_ = sd_ptr;
  }
  bool Init(std::string file_name) {
    unsigned int counter = 0;
    std::string log_name = file_name + std::to_string(counter) + LOG_EXT_;
    while ((sd_ptr_->exists(log_name.c_str())) && (counter < std::numeric_limits<unsigned int>::max())) {
      counter++;
      log_name = file_name + std::to_string(counter) + LOG_EXT_;
    }
    if (sd_ptr_->exists(log_name.c_str())) {
      return false;
    }
    file_ = sd_ptr_->open(log_name, O_RDWR | O_CREAT);
    if (!file_) {
      return false;
    }
    return true;
  }
  unsigned int Write(uint8_t *data, unsigned int bytes) {
    /* Push data onto the FIFO */
    atomic_signal_fence(std::memory_order_acq_rel);
    unsigned int bytes_written = fifo_buffer_.Write(data, bytes);
    atomic_signal_fence(std::memory_order_acq_rel);
    return bytes_written;
  }
  void Flush() {
    atomic_signal_fence(std::memory_order_acq_rel);
    unsigned int size = fifo_buffer_.Size();
    atomic_signal_fence(std::memory_order_acq_rel);
    if (size >= BLOCK_DIM_) {
      /* Pop data off the FIFO */
      atomic_signal_fence(std::memory_order_acq_rel);
      fifo_buffer_.Read(block_buffer_, BLOCK_DIM_);
      atomic_signal_fence(std::memory_order_acq_rel);
      /* Write to SD */
      file_.write(block_buffer_, BLOCK_DIM_);
      file_.sync();
    }
  }
  void Close();

 private:
  /* Types */
  enum Types : uint16_t {
    TYPE_ACCEL,
    TYPE_GYRO,
    TYPE_IMU,
    TYPE_MAG,
    TYPE_PRESSURE,
    TYPE_TEMPERATURE
  };
  /* SD card */
  SdFatSdioEX *sd_ptr_;
  /* SD file */
  File file_;
  /* Log extension */
  const std::string LOG_EXT_ = ".bfs";
  /* Block dimension */
  static const unsigned int BLOCK_DIM_ = 512;
  /* Block buffer */
  uint8_t block_buffer_[BLOCK_DIM_] __attribute__((aligned(4))) = {};
  /* FIFO buffer */
  CircularBuffer<BLOCK_DIM_ * FIFO_DEPTH> fifo_buffer_;
};

#endif  // INCLUDE_LOGGER_LOGGER_H_
