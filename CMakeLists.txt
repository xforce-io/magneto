cmake_minimum_required(VERSION 2.6.4)

project(magneto)

include_directories(
  /usr/include/
  ${CMAKE_SOURCE_DIR}/deps/public-cpp/include/
  /usr/local/include
)

link_directories(
)

set(MAG_OUTPUT_PATH magneto/)
set(MAG_LIB_OUTPUT_PATH  ${MAG_OUTPUT_PATH}/lib/magneto)
set(MAG_INC_OUTPUT_PATH  ${MAG_OUTPUT_PATH}/include/)
set(MAG_INC_OUTPUT_PATH_MAGNETO  ${MAG_INC_OUTPUT_PATH}/magneto/)
set(MAG_INC_OUTPUT_PATH_PUBLIC_CPP  ${MAG_INC_OUTPUT_PATH}/deps/public-cpp/include/public-cpp/)

set(MAG_SRC_DIR "${CMAKE_SOURCE_DIR}/src/")
set(MAG_SRC_DIR_PUBLIC_CPP "${CMAKE_SOURCE_DIR}/deps/public-cpp/include/public-cpp/")

FILE(GLOB_RECURSE MAG_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -W -Wall -Wno-deprecated -Wno-invalid-offsetof -Wno-strict-aliasing -O2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -W -Wall -Wno-deprecated -Wno-invalid-offsetof -Wno-strict-aliasing -DMAGNETO_THRIFT_SUPPORT -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -W -Wall -Wno-deprecated -Wno-invalid-offsetof -Wno-strict-aliasing -DMAGNETO_THRIFT_SUPPORT -DMAGNETO_PROTOBUF_SUPPORT")
set(LIB_DEPS ${CMAKE_SOURCE_DIR}/deps/public-cpp/lib/public-cpp/libpublic-cpp.a)

add_library(magneto
  ${MAG_FILES})

target_link_libraries(magneto
  ${LIB_DEPS}
  log4cplus
  pthread
  rt
  z
  protobuf
  thrift
  #tcmalloc
  #monitor
  )

add_subdirectory(examples)

add_custom_command(TARGET magneto
  PRE_BUILD

  #make magneto
  COMMAND mkdir -p ${MAG_OUTPUT_PATH}
  COMMAND mkdir -p ${MAG_LIB_OUTPUT_PATH} 
  COMMAND mkdir -p ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols/ping
  COMMAND mkdir -p ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols/rapid
  COMMAND mkdir -p ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols/redis
  COMMAND mkdir -p ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols/thrift
  COMMAND mkdir -p ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols/protobuf
  COMMAND mkdir -p ${MAG_INC_OUTPUT_PATH_PUBLIC_CPP}/time

  COMMAND cp ${MAG_SRC_DIR}/magneto.h ${MAG_INC_OUTPUT_PATH_MAGNETO}
  COMMAND cp ${MAG_SRC_DIR}/handlers.h ${MAG_INC_OUTPUT_PATH_MAGNETO}
  COMMAND cp ${MAG_SRC_DIR}/public_model.h ${MAG_INC_OUTPUT_PATH_MAGNETO}
  COMMAND cp ${MAG_SRC_DIR}/error_no.h ${MAG_INC_OUTPUT_PATH_MAGNETO}
  COMMAND cp ${MAG_SRC_DIR}/protocols/protocol.h ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols
  COMMAND cp ${MAG_SRC_DIR}/protocols/protocols.h ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols
  COMMAND cp ${MAG_SRC_DIR}/protocols/ping/protocol_ping.h ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols/ping/
  COMMAND cp ${MAG_SRC_DIR}/protocols/rapid/protocol_rapid.h ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols/rapid/
  COMMAND cp ${MAG_SRC_DIR}/protocols/redis/protocol_redis.h ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols/redis/
  COMMAND cp ${MAG_SRC_DIR}/protocols/thrift/protocol_thrift.h ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols/thrift/
  COMMAND cp ${MAG_SRC_DIR}/protocols/thrift/t_simple_protocol.h ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols/thrift/
  COMMAND cp ${MAG_SRC_DIR}/protocols/protobuf/protocol_protobuf.h ${MAG_INC_OUTPUT_PATH_MAGNETO}/protocols/protobuf/

  COMMAND cp ${MAG_SRC_DIR_PUBLIC_CPP}/common.h ${MAG_INC_OUTPUT_PATH_PUBLIC_CPP}
  COMMAND cp ${MAG_SRC_DIR_PUBLIC_CPP}/io_helper.h ${MAG_INC_OUTPUT_PATH_PUBLIC_CPP}
  COMMAND cp ${MAG_SRC_DIR_PUBLIC_CPP}/slice.hpp ${MAG_INC_OUTPUT_PATH_PUBLIC_CPP}
  COMMAND cp ${MAG_SRC_DIR_PUBLIC_CPP}/buffer.hpp ${MAG_INC_OUTPUT_PATH_PUBLIC_CPP}
  COMMAND cp ${MAG_SRC_DIR_PUBLIC_CPP}/time/time.h ${MAG_INC_OUTPUT_PATH_PUBLIC_CPP}/time/
  COMMAND cp ${MAG_SRC_DIR_PUBLIC_CPP}/gmonitor.h ${MAG_INC_OUTPUT_PATH_PUBLIC_CPP}
  COMMAND cp ${MAG_SRC_DIR_PUBLIC_CPP}/mem_profile.h ${MAG_INC_OUTPUT_PATH_PUBLIC_CPP}
)

add_custom_command(TARGET magneto
  POST_BUILD

  #make magneto
  COMMAND cp libmagneto.a ${MAG_LIB_OUTPUT_PATH}
  COMMAND cp ${CMAKE_SOURCE_DIR}/deps/public-cpp/lib/public-cpp/libpublic-cpp.a ${MAG_LIB_OUTPUT_PATH}
)
