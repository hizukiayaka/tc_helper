cmake_minimum_required(VERSION 2.6)

PROJECT(xcostc C)

option(XCOSTC_DEBUG "Enable Debug in xcostc" OFF)

IF(XCOSTC_DEBUG)
	ADD_DEFINITIONS(-Os -Wall --std=gnu99 -g3 -Wmissing-declarations -DXCOSTC_DEBUG)
ELSE(XCOSTC_DEBUG)
	ADD_DEFINITIONS(-Os -Wall --std=gnu99 -Wmissing-declarations)
ENDIF(XCOSTC_DEBUG)


ADD_EXECUTABLE(xcostc network.c main.c options.c tc.c)
TARGET_LINK_LIBRARIES(xcostc pthread uci ubox m)

INSTALL(TARGETS xcostc RUNTIME DESTINATION sbin)
