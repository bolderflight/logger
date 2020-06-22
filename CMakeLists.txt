cmake_minimum_required(VERSION 3.13)
if (DEFINED MCU)
	# Setting up the toolchain
	set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/cortex.cmake")
endif()
# Project information
project(Logger
	VERSION 1.0
	DESCRIPTION "Generic data logger"
	LANGUAGES CXX
)
if (DEFINED MCU)
	# Grab the processor and set up definitions and compile options
	include(${CMAKE_SOURCE_DIR}/cmake/config_mcu.cmake)
	configMcu(${MCU})
	include(FetchContent)
	# Fetch sd
	FetchContent_Declare(
		sd
		GIT_REPOSITORY 	git@gitlab.com:bolderflight/software/sd.git
	)
	FetchContent_MakeAvailable(sd)
	# Fetch circular buffer
	FetchContent_Declare(
		circle_buf
		GIT_REPOSITORY 	git@gitlab.com:bolderflight/software/circle_buf.git
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
endif()
# Example and test if this project is built separately
if (PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
	if (DEFINED MCU)
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
		include(${CMAKE_SOURCE_DIR}/cmake/flash_mcu.cmake)
		FlashMcu(logger_example ${MCU})
	endif()
endif()
