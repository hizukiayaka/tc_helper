cmake_minimum_required(VERSION 2.6)

PROJECT(xcostc C)

ADD_DEFINITIONS(-Os -Wall --std=gnu99 -g3 -Wmissing-declarations)

ADD_EXECUTABLE(xcostc network.c main.c options.c tc.c)
TARGET_LINK_LIBRARIES(xcostc pthread uci ubox)

INSTALL(TARGETS xcostc RUNTIME DESTINATION sbin)
