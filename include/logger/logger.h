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

#ifndef INCLUDE_LOGGER_LOGGER_H_
#define INCLUDE_LOGGER_LOGGER_H_

#include <cstdint>
#include <string>
#include <atomic>
#include <limits>
#include "sd/SdFat.h"
#include "circle_buf/circle_buf.h"

namespace bfs {

template<std::size_t FIFO_DEPTH>
class Logger {
 public:
  explicit Logger(SdFat32 *sd) : sd_(sd) {}
  int Init(std::string file_name) {
    std::size_t counter = 0;
    std::string log_name = file_name + std::to_string(counter) + LOG_EXT_;
    while ((sd_->exists(log_name.c_str())) && (counter < std::numeric_limits<std::size_t>::max())) {
      counter++;
      log_name = file_name + std::to_string(counter) + LOG_EXT_;
    }
    if (sd_->exists(log_name.c_str())) {
      return -1;
    }
    file_ = sd_->open(log_name, O_RDWR | O_CREAT);
    if (!file_) {
      return -1;
    }
    return counter;
  }
  std::size_t Write(uint8_t *data, std::size_t bytes) {
    /* Push data onto the FIFO */
    atomic_signal_fence(std::memory_order_acq_rel);
    std::size_t bytes_written = fifo_buffer_.Write(data, bytes);
    atomic_signal_fence(std::memory_order_acq_rel);
    return bytes_written;
  }
  void Flush() {
    atomic_signal_fence(std::memory_order_acq_rel);
    std::size_t size = fifo_buffer_.Size();
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
  void Close() {
    /* Write any logs still in the buffer */
    atomic_signal_fence(std::memory_order_acq_rel);
    std::size_t size = fifo_buffer_.Size();
    atomic_signal_fence(std::memory_order_acq_rel);
    while (size >= BLOCK_DIM_) {
      Flush();
      atomic_signal_fence(std::memory_order_acq_rel);
      size = fifo_buffer_.Size();
      atomic_signal_fence(std::memory_order_acq_rel);
    }
    /* Write any partial blocks left */
    atomic_signal_fence(std::memory_order_acq_rel);
    size = fifo_buffer_.Size();
    atomic_signal_fence(std::memory_order_acq_rel);
    if (size) {
      /* Pop data off the FIFO */
      atomic_signal_fence(std::memory_order_acq_rel);
      std::size_t bytes_to_write = fifo_buffer_.Read(block_buffer_, BLOCK_DIM_);
      atomic_signal_fence(std::memory_order_acq_rel);
      /* Write data */
      file_.write(block_buffer_, bytes_to_write);
      file_.sync();
    }
    /* Close out the file */
    file_.close();
  }

 private:
  /* SD card */
  SdFat32 *sd_;
  /* SD file */
  File32 file_;
  /* Log extension */
  const std::string LOG_EXT_ = ".bfs";
  /* Block dimension */
  static const std::size_t BLOCK_DIM_ = 512;
  /* Block buffer */
  uint8_t block_buffer_[BLOCK_DIM_] __attribute__((aligned(4))) = {};
  /* FIFO buffer */
  CircularBuffer<uint8_t, BLOCK_DIM_ * FIFO_DEPTH> fifo_buffer_;
};

}  // namespace bfs

#endif  // INCLUDE_LOGGER_LOGGER_H_
