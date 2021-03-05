# Logger
This library implements a generic data logger.
   * [License](LICENSE.md)
   * [Changelog](CHANGELOG.md)
   * [Contributing guide](CONTRIBUTING.md)

## Installation
CMake is used to build this library, which is exported as a library target called *logger*. The header is added as:

```
#include "logger/logger.h"
```

The library can be also be compiled stand-alone using the CMake idiom of creating a *build* directory and then, from within that directory issuing:

```
cmake .. -DMCU=MK66FX1M0
make
```

This will build the library and an example executable called *logger_example*. The example executable source files are located at *examples/logger_example.cc*. Notice that the *cmake* command includes a define specifying the microcontroller the code is being compiled for. This is required to correctly configure the code, CPU frequency, and compile/linker options. The available MCUs are:
   * MK20DX128
   * MK20DX256
   * MK64FX512
   * MK66FX1M0
   * MKL26Z64
   * IMXRT1062_T40
   * IMXRT1062_T41

These are known to work with the same packages used in Teensy products. Also switching packages is known to work well, as long as it's only a package change. 

# Namespace
This library is in namespace *bfs*

## Methods

**Logger<FIFO_DEPTH>(SdFat32 &ast;sd)** Creates a Logger object given a pointer to the *SdFat32* class, which is used to control the SD card. A template parameter is used to specify the FIFO buffer depth. A larger FIFO depth will help prevent missing data logging frames when the SD card experiences latency during write operations. Memory is allocated in 512 byte chunks at the specified depth; for example, if the FIFO depth is set to 200, a total of 102,400 bytes will be allocated.

```C++
SdFat32 sd;
bfs::Logger<200> datalog(&sd);
```

**int Init(std::string file_name)** Initializes the data log given a file name. The .bfs extension is always used and the file name is appended with a number to prevent overwriting existing files. The log file number is returned on success and a -1 is returned on failure.

```C++
bool status = datalog.Init("test_data");
```

**std::size_t Write(uint8_t &ast;data, std::size_t bytes)** Adds bytes to the FIFO buffer given a pointer to the data and the number of bytes to write. Returns the number of bytes added. 

```C++
unsigned int counter = 0;
datalog.Write((uint8_t *)&counter, sizeof(counter));
```

**void Flush()** Writes the FIFO buffer to the SD card. This method should be called in a low priority loop to continuously write data to the SD card, while enabling high priority interrupts to collect new data and add it to the buffer.

```C++
while(1) {
   datalog.Flush();
}
```

**void Close()** Writes any remaining data to the SD card and closes the data logging file that was created.

```C++
datalog.Close();
```
