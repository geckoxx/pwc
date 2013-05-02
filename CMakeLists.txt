project(pwc)

SET(CMAKE_CXX_FLAGS "-std=c++0x -pthread")

add_executable(pwc main.cpp)

install(TARGETS pwc RUNTIME DESTINATION bin)
