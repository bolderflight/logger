# v3.14 required for FetchContent_MakeAvailable
cmake_minimum_required(VERSION 3.14)
if (DEFINED MCU)
  include(FetchContent)
  FetchContent_Declare(
    mcu_support
    GIT_REPOSITORY https://github.com/bolderflight/mcu-support.git
    GIT_TAG v1.0.0
  )
  FetchContent_MakeAvailable(mcu_support)
  # Setting up the toolchain
  set(CMAKE_TOOLCHAIN_FILE "${mcu_support_SOURCE_DIR}/cmake/cortex.cmake")
  # Project information
  project(Logger
    VERSION 2.1.0
    DESCRIPTION "Generic data logger"
    LANGUAGES CXX
  )
  # Grab the processor and set up definitions and compile options
  include(${mcu_support_SOURCE_DIR}/cmake/config_mcu.cmake)
  configMcu(${MCU} ${mcu_support_SOURCE_DIR})
  # Fetch sd
  FetchContent_Declare(
    sd
    GIT_REPOSITORY 	https://github.com/bolderflight/sd.git
    GIT_TAG v2.1.0
  )
  FetchContent_MakeAvailable(sd)
  # Fetch circular buffer
  FetchContent_Declare(
    circle_buf
    GIT_REPOSITORY 	https://github.com/bolderflight/circle_buf.git
    GIT_TAG v4.0.3
  )
  FetchContent_MakeAvailable(circle_buf)
  # Add the library target
  add_library(logger INTERFACE)
  # Link libraries
  target_link_libraries(logger
    INTERFACE
      sd
      circle_buf
  )
  # Setup include directories 
  target_include_directories(logger INTERFACE include/)

  # Example and test if this project is built separately
  if (PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
    # Add the example target
    add_executable(logger_example examples/logger_example.cc)
    # Add the includes
    target_include_directories(logger_example PUBLIC 
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
    )
    # Link libraries to the example target
    target_link_libraries(logger_example
      PRIVATE 
        logger
    )
    # Add hex and upload targets
    include(${mcu_support_SOURCE_DIR}/cmake/flash_mcu.cmake)
    FlashMcu(logger_example ${MCU} ${mcu_support_SOURCE_DIR})
  endif()
endif()
