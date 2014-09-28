cmake_minimum_required(VERSION 2.8.8)

include_directories(
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
set(PINGPONG_PRJ_PATH ../../examples/pingpong/)
set(CLUSTER_AGENT_PRJ_PATH ../../examples/cluster_agent/)
set(BROADCAST_PRJ_PATH ../../examples/broadcast/)

add_executable(pingpong/bin/pingpong
  pingpong/src/pingpong.cpp)

add_executable(cluster_agent/bin/cluster_agent
  cluster_agent/src/cluster_agent.cpp)

add_executable(broadcast/bin/broadcast
  broadcast/src/broadcast.cpp)

target_link_libraries(pingpong/bin/pingpong
  log4cplus
  pthread
  rt
  z)

target_link_libraries(cluster_agent/bin/cluster_agent
  log4cplus
  pthread
  rt
  z)

target_link_libraries(broadcast/bin/broadcast
  log4cplus
  pthread
  rt
  z)

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

add_custom_command(TARGET cluster_agent/bin/cluster_agent
  PRE_BUILD

  #make examples
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/cluster_agent/
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/cluster_agent/conf
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/cluster_agent/bin
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EXP_OUTPUT_PATH}/cluster_agent/log
  COMMAND cp -rf ${CLUSTER_AGENT_PRJ_PATH}/conf/* ${EXP_OUTPUT_PATH}/cluster_agent/conf
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