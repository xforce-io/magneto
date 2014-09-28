cmake_minimum_required(VERSION 2.8.8)

project(magneto)

include_directories(
  /usr/local/include/
  )

set(MAG_OUTPUT_PATH magneto/)
set(MAG_LIB_OUTPUT_PATH  ${MAG_OUTPUT_PATH}/lib/magneto)
set(MAG_INC_OUTPUT_PATH  ${MAG_OUTPUT_PATH}/include/magneto)

set(MAG_SRC_DIR ../src/)

FILE(GLOB_RECURSE MAG_FILES "src/*.cpp")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -W -Wall -Wno-deprecated -Wno-invalid-offsetof -Wno-strict-aliasing -O2")

add_library(magneto
  ${MAG_FILES})

target_link_libraries(magneto
  pthread
  rt
  z)

add_subdirectory(examples)

add_custom_command(TARGET magneto
  PRE_BUILD

  #make magneto
  COMMAND ${CMAKE_COMMAND} -E make_directory ${MAG_OUTPUT_PATH} 
  COMMAND ${CMAKE_COMMAND} -E make_directory ${MAG_LIB_OUTPUT_PATH} 
  COMMAND ${CMAKE_COMMAND} -E make_directory ${MAG_INC_OUTPUT_PATH} 
  COMMAND ${CMAKE_COMMAND} -E make_directory ${MAG_INC_OUTPUT_PATH}/public/
  COMMAND ${CMAKE_COMMAND} -E make_directory ${MAG_INC_OUTPUT_PATH}/protocols/ping
  COMMAND ${CMAKE_COMMAND} -E make_directory ${MAG_INC_OUTPUT_PATH}/protocols/rapid
  COMMAND ${CMAKE_COMMAND} -E make_directory ${MAG_INC_OUTPUT_PATH}/protocols/redis
  COMMAND ${CMAKE_COMMAND} -E make_directory ${MAG_INC_OUTPUT_PATH}/public/time

  COMMAND ${CMAKE_COMMAND} -E copy ${MAG_SRC_DIR}/magneto.h ${MAG_INC_OUTPUT_PATH}
  COMMAND ${CMAKE_COMMAND} -E copy ${MAG_SRC_DIR}/handlers.h ${MAG_INC_OUTPUT_PATH}
  COMMAND ${CMAKE_COMMAND} -E copy ${MAG_SRC_DIR}/error_no.h ${MAG_INC_OUTPUT_PATH}
  COMMAND ${CMAKE_COMMAND} -E copy ${MAG_SRC_DIR}/public/common.h ${MAG_INC_OUTPUT_PATH}/public/
  COMMAND ${CMAKE_COMMAND} -E copy ${MAG_SRC_DIR}/public/buffer.hpp ${MAG_INC_OUTPUT_PATH}/public/
  COMMAND ${CMAKE_COMMAND} -E copy ${MAG_SRC_DIR}/public/time/time.h ${MAG_INC_OUTPUT_PATH}/public/time/
  COMMAND ${CMAKE_COMMAND} -E copy ${MAG_SRC_DIR}/protocols/protocol.h ${MAG_INC_OUTPUT_PATH}/protocols
  COMMAND ${CMAKE_COMMAND} -E copy ${MAG_SRC_DIR}/protocols/protocols.h ${MAG_INC_OUTPUT_PATH}/protocols

  COMMAND ${CMAKE_COMMAND} -E copy
      ${MAG_SRC_DIR}/protocols/ping/protocol_ping.h
      ${MAG_INC_OUTPUT_PATH}/protocols/ping/

  COMMAND ${CMAKE_COMMAND} -E copy
      ${MAG_SRC_DIR}/protocols/rapid/protocol_rapid.h
      ${MAG_INC_OUTPUT_PATH}/protocols/rapid/

  COMMAND ${CMAKE_COMMAND} -E copy
      ${MAG_SRC_DIR}/protocols/redis/protocol_redis.h
      ${MAG_INC_OUTPUT_PATH}/protocols/redis/
)

add_custom_command(TARGET magneto
  POST_LINK

  #make magneto
  COMMAND mv libmagneto.a ${MAG_LIB_OUTPUT_PATH}
)
