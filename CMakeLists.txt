cmake_minimum_required(VERSION 2.6)

project(CommConnection)
SET(GCC_COMPILE_FLAGS "-Wall -std=c++11 -O3")
SET(GCC_LINKER_FLAGS "-lpthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLALGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINKER_FLAGS}")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GCC_LINKER_FLAGS}")
SET(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GCC_LINKER_FLAGS}")

include_directories("${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/src/Linux" "${PROJECT_SOURCE_DIR}/src/Windows")
add_subdirectory(src)
add_subdirectory(src/Linux)
add_subdirectory(src/Windows)

add_library(LinuxCommConnection SHARED src/CommConnection.cpp src/NetworkConnection.cpp src/SerialConnection.cpp)
set_target_properties(LinuxCommConnection PROPERTIES OUTPUT_NAME LinuxCommConnection)

add_library(LinuxCommConnectionStatic STATIC src/CommConnection.cpp src/NetworkConnection.cpp src/SerialConnection.cpp)
#set_target_properties(LinuxCommConnectionStatic PROPERTIES OUTPUT_NAME LinuxCommConnectionStatic)

add_executable(CommConnectionTest tests/CommConnectionTest.cpp)
target_link_libraries(CommConnectionTest LinuxCommConnection)

install(TARGETS LinuxCommConnection DESTINATION /usr/lib)
install(FILES src/CommConnection.h src/NetworkConnection.h src/SerialConnection.h DESTINATION /usr/include/LinuxCommConnection)
