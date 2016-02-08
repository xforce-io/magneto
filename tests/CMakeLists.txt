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


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -W -Wall -Wno-deprecated -Wno-invalid-offsetof -Wno-strict-aliasing -O2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -W -Wall -Wno-deprecated -Wno-invalid-offsetof -Wno-strict-aliasing -DMAGNETO_THRIFT_SUPPORT -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -W -Wall -Wno-deprecated -Wno-invalid-offsetof -Wno-strict-aliasing -DMAGNETO_PROTOBUF_SUPPORT -Dprivate=public -Dprotected=public")

FILE(GLOB_RECURSE MAG_SRC_FILES "${CMAKE_SOURCE_DIR}/../src/*.cpp")

set(TEST_PROTOBUF_FILES 
  "${CMAKE_SOURCE_DIR}/src/protocols/protobuf/test_protobuf.cpp"
  "${CMAKE_SOURCE_DIR}/src/protocols/protobuf/sample.pb.cc"
)

add_executable(test_protobuf
  ${TEST_PROTOBUF_FILES}
  ${MAG_SRC_FILES})

target_link_libraries(test_protobuf
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
