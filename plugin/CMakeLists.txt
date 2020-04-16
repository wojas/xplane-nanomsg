#
# Copyright 2018 Capitar IT Group BV <info@capitar.com>
# Copyright 2018 Staysail Systems, Inc. <info@staysail.tech>
#
# This software is supplied under the terms of the MIT License, a
# copy of which should be located in the distribution where this
# file was obtained (LICENSE.txt).  A copy of the license may also be
# found online at https://opensource.org/licenses/MIT.

cmake_minimum_required (VERSION 3.16.1)

project(reqrep)


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
#include_directories(protobuf/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS foo.proto)
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS EXPORT_MACRO DLL_EXPORT foo.proto)
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS DESCRIPTORS PROTO_DESCS foo.proto)
#protobuf_generate_python(PROTO_PY foo.proto)
#add_executable(bar bar.cc ${PROTO_SRCS} ${PROTO_HDRS})
#target_link_libraries(bar ${PROTOBUF_LIBRARIES})

# TODO: Protobuf demos - REMOVE
protobuf_generate_cpp(PROTO_DEMO_SRCS PROTO_DEMO_HDRS addressbook.proto)
add_executable(add_person add_person.cc ${PROTO_DEMO_SRCS} ${PROTO_DEMO_HDRS})
target_link_libraries(add_person PRIVATE ${Protobuf_LIBRARIES})
add_executable(list_people list_people.cc ${PROTO_DEMO_SRCS} ${PROTO_DEMO_HDRS})
target_link_libraries(list_people PRIVATE ${Protobuf_LIBRARIES})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/xplane.proto)

#find_package(nng CONFIG REQUIRED)
add_subdirectory(nng)

add_executable(reqrep reqrep.c)
#add_executable(regrep regrep.c ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(reqrep PRIVATE nng ${Protobuf_LIBRARIES})


add_subdirectory(xplane_sdk)
add_xplane_plugin(hello-world 301 hello-world.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(hello-world PRIVATE nng ${Protobuf_LIBRARIES})
