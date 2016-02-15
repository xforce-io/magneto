cmake_minimum_required(VERSION 2.6.4)

project(test_magneto)

include_directories(
  /usr/include/
  /usr/local/include
  ${CMAKE_SOURCE_DIR}/../deps/public-cpp/include/
)

link_directories(
  /usr/local/lib/
  /usr/local/lib64/
  ${CMAKE_SOURCE_DIR}/../deps/public-cpp/lib/public-cpp
)

link_libraries(
  ${LIB_DEPS}
  log4cplus
  pthread
  rt
  z
  protobuf
  gtest
  public-cpp
  #thrift
  #tcmalloc
  #monitor
  )

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -W -Wall -Wno-deprecated -Wno-invalid-offsetof -Wno-strict-aliasing -O2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -W -Wall -Wno-deprecated -Wno-invalid-offsetof -Wno-strict-aliasing -DMAGNETO_THRIFT_SUPPORT -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -W -Wall -Wno-deprecated -Wno-invalid-offsetof -Wno-strict-aliasing -DMAGNETO_PROTOBUF_SUPPORT -Dprivate=public -Dprotected=public")

FILE(GLOB_RECURSE MAG_SRC_FILES "${CMAKE_SOURCE_DIR}/../src/*.cpp")

add_library(magneto
  ${MAG_SRC_FILES}
)

add_executable(test_confs
  "${CMAKE_SOURCE_DIR}/src/confs/test_confs.cpp"
)
add_dependencies(test_confs magneto)
target_link_libraries(test_confs magneto)

add_executable(test_public
  "${CMAKE_SOURCE_DIR}/src/confs/test_public.cpp"
)
add_dependencies(test_public magneto)
target_link_libraries(test_public magneto)

add_executable(test_reload_confs
  "${CMAKE_SOURCE_DIR}/src/confs/test_reload_confs.cpp"
)
add_dependencies(test_reload_confs magneto)
target_link_libraries(test_reload_confs magneto)

add_executable(test_conns_mgr
  "${CMAKE_SOURCE_DIR}/src/conns_mgr/test_conns_mgr.cpp"
)
add_dependencies(test_conns_mgr magneto)
target_link_libraries(test_conns_mgr magneto)

set(TEST_PROTOBUF_FILES 
  "${CMAKE_SOURCE_DIR}/src/protocols/protobuf/test_protobuf.cpp"
  "${CMAKE_SOURCE_DIR}/src/protocols/protobuf/sample.pb.cc"
)

add_executable(test_protobuf
  ${TEST_PROTOBUF_FILES}
)
add_dependencies(test_protobuf magneto)
target_link_libraries(test_protobuf magneto)

add_executable(test_redis_parser
  "${CMAKE_SOURCE_DIR}/src/protocols/redis/test_redis_parser.cpp"
)
add_dependencies(test_redis_parser magneto)
target_link_libraries(test_redis_parser magneto)

add_executable(test_pool_protocols
  "${CMAKE_SOURCE_DIR}/src/protocols/test_pool_protocols.cpp"
)
add_dependencies(test_pool_protocols magneto)
target_link_libraries(test_pool_protocols magneto)

add_executable(test_ctx_helper
  "${CMAKE_SOURCE_DIR}/src/test_ctx_helper.cpp"
)
add_dependencies(test_ctx_helper magneto)
target_link_libraries(test_ctx_helper magneto)

add_executable(test_cluster_agent
  "${CMAKE_SOURCE_DIR}/src/magneto/test_cluster_agent.cpp"
)
add_dependencies(test_cluster_agent magneto)
target_link_libraries(test_cluster_agent magneto)

add_executable(test_pingpong
  "${CMAKE_SOURCE_DIR}/src/magneto/test_pingpong.cpp"
)
add_dependencies(test_pingpong magneto)
target_link_libraries(test_pingpong magneto)

add_executable(test_broadcast
  "${CMAKE_SOURCE_DIR}/src/magneto/test_broadcast.cpp"
)
add_dependencies(test_broadcast magneto)
target_link_libraries(test_broadcast magneto)
