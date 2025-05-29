cmake_minimum_required(VERSION 3.30.0)
project(OSMParser VERSION 0.1.0 LANGUAGES C CXX)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(EXECUTABLE_OUTPUT_PATH_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin/release")
set(EXECUTABLE_OUTPUT_PATH_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin/debug")
set(BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/binBuild )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/debug) 

add_executable(${PROJECT_NAME} main.cpp)

set(CMAKE_CXX_STANDARD 20)
find_package(ZLIB REQUIRED)
find_package(Protobuf REQUIRED) 
find_package(absl REQUIRED)
find_package(SQLite3 REQUIRED)
 

 
message(STATUS "Building Project ${CMAKE_PROJECT_NAME}") 
message(STATUS "ZLIB INCL DIR: ${ZLIB_INCLUDE_DIR}")
message(STATUS "Using Protocol Buffers ${Protobuf_VERSION}")
message(STATUS "Using Protocol Buffers Include DIR ${Protobuf_INCLUDE_DIR}")

 
target_precompile_headers(${CMAKE_PROJECT_NAME} PRIVATE src/pch.h) 


target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE  "${CMAKE_CURRENT_SOURCE_DIR}/src") 
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE  "${ZLIB_INCLUDE_DIR}") 
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE  "${Protobuf_INCLUDE_DIR}") 
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE  "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/rapidjson/include")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src")   
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/rapidjson") 
   
target_link_libraries(${CMAKE_PROJECT_NAME} ZLIB::ZLIB)
target_link_libraries(${CMAKE_PROJECT_NAME} protobuf::libprotobuf)
target_link_libraries(${CMAKE_PROJECT_NAME}
        absl::base
        absl::synchronization
        absl::strings
        absl::memory
        absl::time
        absl::log_internal_message
        absl::log_internal_check_op)
target_link_libraries(${CMAKE_PROJECT_NAME} SQLite::SQLite3)

 


include(CTest)   
enable_testing() 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DFV_DEBUG")


#bset(CMAKE_BUILD_TYPE Debug) 
include(CPack) 
 