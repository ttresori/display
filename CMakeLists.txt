cmake_minimum_required(VERSION 4.0)

include(pico_sdk_import.cmake)

project(display)

pico_sdk_init()

set(	SRC
	src/main.c
)

set( HEADER
     include/lib.h
)

add_executable(display ${SRC} ${HEADER})

target_link_libraries(display pico_stdlib hardware_i2c)

target_include_directories(display
	PRIVATE
	include
)

pico_add_extra_outputs(display)
