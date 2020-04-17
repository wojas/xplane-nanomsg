#
# Copyright 2018 Capitar IT Group BV <info@capitar.com>
# Copyright 2018 Staysail Systems, Inc. <info@staysail.tech>
#
# This software is supplied under the terms of the MIT License, a
# copy of which should be located in the distribution where this
# file was obtained (LICENSE.txt).  A copy of the license may also be
# found online at https://opensource.org/licenses/MIT.

# TODO: Figure out the actual minimum version, this one is very new
cmake_minimum_required (VERSION 3.16.1)

project(xplane-nanomsg LANGUAGES CXX C)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# https://cmake.org/cmake/help/latest/module/FindProtobuf.html
set(Protobuf_USE_STATIC_LIBS TRUE)
set(Protobuf_DEBUG TRUE)

# Failed attempt to use cmake for the protobuf build
#add_subdirectory(protobuf/cmake)
#set(Protobuf_DIR ${CMAKE_SOURCE_DIR}/protobuf-install)
#set(Protobuf_INCLUDE_DIR ${Protobuf_DIR}/src)
#set(Protobuf_PROTOC_EXECUTABLE ${CMAKE_BINARY_DIR}/protobuf/cmake/protoc)
#set(Protobuf_LIBRARIES ${CMAKE_BINARY_DIR}/protobuf/cmake/libprotobuf.a)
#set(Protobuf_LITE_LIBRARIES ${CMAKE_BINARY_DIR}/protobuf/cmake/libprotobuf-list.a)

# Depends on ./build.sh having been run
set(Protobuf_DIR ${CMAKE_SOURCE_DIR}/protobuf-install)
set(Protobuf_INCLUDE_DIR ${Protobuf_DIR}/include)
set(Protobuf_PROTOC_EXECUTABLE ${Protobuf_DIR}/bin/protoc)
set(Protobuf_LIBRARIES ${Protobuf_DIR}/lib/libprotobuf.a)
set(Protobuf_LITE_LIBRARIES ${Protobuf_DIR}/lib/libprotobuf-lite.a)
find_package(Protobuf REQUIRED)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/xplane.proto)

#find_package(nng CONFIG REQUIRED)
add_subdirectory(nng)

set(XP_SOURCES Plugin.cpp ${PROTO_SRCS} ${PROTO_HDRS} Publisher.cpp Publisher.h Statistics.cpp Statistics.h)

add_subdirectory(xplane_sdk)
add_xplane_plugin(nanomsg-plugin 301 ${XP_SOURCES})
target_link_libraries(nanomsg-plugin PRIVATE nng ${Protobuf_LIBRARIES})

# Test executable with SDK stubs
add_executable(test-nanomsg-plugin test-stub.cpp ${XP_SOURCES})
add_xplane_sdk_definitions(test-nanomsg-plugin 301)
set(XPLANE_INCLUDES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/xplane_sdk/sdk/CHeaders/" CACHE STRING PATH)
target_include_directories(test-nanomsg-plugin PUBLIC "${XPLANE_INCLUDES_PATH}/XPLM" "${XPLANE_INCLUDES_PATH}/Widgets" "${XPLANE_INCLUDES_PATH}/Wrappers")
target_link_libraries(test-nanomsg-plugin PRIVATE nng ${Protobuf_LIBRARIES})

