cmake_minimum_required(VERSION 4.0)
    
project(display)

set(	SRC
	src/main.c
)

set( HEADER
     include/lib.h
)

add_executable(display ${SRC} ${HEADER})
