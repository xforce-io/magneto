cmake_minimum_required(VERSION 2.6.4)

include_directories(
  /usr/local/include/
  ../src/
  )

link_directories(
  /usr/local/lib/
  magneto/lib/magneto/
)

link_libraries(
  magneto
)

set(EXP_OUTPUT_PATH ./)

############################
#     pingpong
############################
set(PINGPONG_PRJ_PATH ../../examples/pingpong/)

add_executable(pingpong/bin/pingpong
  pingpong/src/pingpong.cpp)

target_link_libraries(pingpong/bin/pingpong
  log4cplus
  pthread
  rt
  z
  #tcmalloc
  )

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
set(CLUSTER_AGENT_PRJ_PATH ../../examples/cluster_agent/)

add_executable(cluster_agent/bin/cluster_agent
  cluster_agent/src/cluster_agent.cpp)

target_link_libraries(cluster_agent/bin/cluster_agent
  log4cplus
  pthread
  rt
  z
  #tcmalloc
  )

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
set(BROADCAST_PRJ_PATH ../../examples/broadcast/)

add_executable(broadcast/bin/broadcast
  broadcast/src/broadcast.cpp)

target_link_libraries(broadcast/bin/broadcast
  log4cplus
  pthread
  rt
  z
  #tcmalloc
  )

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
#set(THRIFT_PROTOCOL_PRJ_PATH ../../examples/thrift_protocol/)
#
#add_executable(thrift_protocol/bin/thrift_protocol
#  thrift_protocol/src/thrift_protocol.cpp
#  thrift_protocol/src/gen-cpp/PingPongTest.cpp
#  thrift_protocol/src/gen-cpp/sample_proto_constants.cpp
#  thrift_protocol/src/gen-cpp/sample_proto_types.cpp)
#
#target_link_libraries(thrift_protocol/bin/thrift_protocol
#  log4cplus
#  pthread
#  rt
#  z
#  #tcmalloc)
#
#add_custom_command(TARGET thrift_protocol/bin/thrift_protocol
#  PRE_BUILD
#
#  #make examples
#  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/thrift_protocol/
#  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/thrift_protocol/conf
#  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/thrift_protocol/bin
#  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/thrift_protocol/log
#  COMMAND cp -rf ${THRIFT_PROTOCOL_PRJ_PATH}/conf/* ${EXP_OUTPUT_PATH}/thrift_protocol/conf
#)
