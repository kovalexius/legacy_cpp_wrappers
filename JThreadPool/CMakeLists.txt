cmake_minimum_required(VERSION 2.8)

project(JThreadPool)

set (JTHREADPOOL_SOURCE 
		JThreadPool/src/CJThreadPool.cpp
		JThreadPool/src/CJThreadPool.h)
		
add_library(jThreadPool STATIC ${JTHREADPOOL_SOURCE})