cmake_minimum_required(VERSION 2.8)

project(pwc)

SET(CMAKE_CXX_FLAGS "-std=c++0x -pthread")

FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
message("OPENMP FOUND")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -D_GLIBCXX_PARALLEL")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(pwc main.cpp)

#target_link_libraries(pwc openmp)

install(TARGETS pwc RUNTIME DESTINATION bin)
