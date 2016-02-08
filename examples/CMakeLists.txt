cmake_minimum_required(VERSION 2.6.4)

include_directories(
  /usr/include/
  /usr/local/include/
  ../src/
  )

link_directories(
  /usr/local/lib/
  /usr/local/lib64/
  magneto/lib/magneto/
)

link_libraries(
  magneto
)

set(EXP_OUTPUT_PATH ./)

############################
#     pingpong
############################
set(PINGPONG_PRJ_PATH "${CMAKE_CURRENT_SOURCE_DIR}/pingpong/")

add_executable(pingpong/bin/pingpong
  pingpong/src/pingpong.cpp)

add_custom_command(TARGET pingpong/bin/pingpong  
  PRE_BUILD

  #make examples
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH} 
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/pingpong 
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/pingpong/conf
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/pingpong/bin
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/pingpong/log
  COMMAND cp -rf ${PINGPONG_PRJ_PATH}/conf/* ${EXP_OUTPUT_PATH}/pingpong/conf
)

############################
#     cluster agent
############################
set(CLUSTER_AGENT_PRJ_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cluster_agent/")

add_executable(cluster_agent/bin/cluster_agent
  cluster_agent/src/cluster_agent.cpp)

add_custom_command(TARGET cluster_agent/bin/cluster_agent
  PRE_BUILD

  #make examples
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/cluster_agent/
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/cluster_agent/conf
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/cluster_agent/bin
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/cluster_agent/log
  COMMAND cp -rf ${CLUSTER_AGENT_PRJ_PATH}/conf/* ${EXP_OUTPUT_PATH}/cluster_agent/conf
)

############################
#     broadcast
############################
set(BROADCAST_PRJ_PATH "${CMAKE_CURRENT_SOURCE_DIR}/broadcast/")

add_executable(broadcast/bin/broadcast
  broadcast/src/broadcast.cpp)

add_custom_command(TARGET broadcast/bin/broadcast
  PRE_BUILD

  #make examples
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/broadcast/
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/broadcast/conf
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/broadcast/bin
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/broadcast/log
  COMMAND cp -rf ${BROADCAST_PRJ_PATH}/conf/* ${EXP_OUTPUT_PATH}/broadcast/conf
)

#############################
##     thrift protocol 
############################
set(THRIFT_PROTOCOL_PRJ_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thrift_protocol/")

add_executable(thrift_protocol/bin/thrift_protocol
  thrift_protocol/src/thrift_protocol.cpp
  thrift_protocol/src/gen-cpp/PingPongTest.cpp
  thrift_protocol/src/gen-cpp/sample_proto_constants.cpp
  thrift_protocol/src/gen-cpp/sample_proto_types.cpp)

add_custom_command(TARGET thrift_protocol/bin/thrift_protocol
  PRE_BUILD

  #make examples
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/thrift_protocol/
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/thrift_protocol/conf
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/thrift_protocol/bin
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/thrift_protocol/log
  COMMAND cp -rf ${THRIFT_PROTOCOL_PRJ_PATH}/conf/* ${EXP_OUTPUT_PATH}/thrift_protocol/conf
)

#############################
##     pb protocol 
############################
set(PB_PROTOCOL_PRJ_PATH "${CMAKE_CURRENT_SOURCE_DIR}/pb_protocol/")

add_executable(pb_protocol/bin/pb_protocol
  pb_protocol/src/pb_protocol.cpp
  pb_protocol/src/sample.pb.cc)

add_custom_command(TARGET pb_protocol/bin/pb_protocol
  PRE_BUILD

  #make examples
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/pb_protocol/
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/pb_protocol/conf
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/pb_protocol/bin
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/pb_protocol/log
  COMMAND cp -rf ${PB_PROTOCOL_PRJ_PATH}/conf/* ${EXP_OUTPUT_PATH}/pb_protocol/conf
)
