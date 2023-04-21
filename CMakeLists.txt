cmake_minimum_required(VERSION 3.5)
project(MyProject)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)

# Find boost library 
SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system thread)

IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} include)
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

# Add source files
file(GLOB SOURCES "src/*.cpp")


# Add compiler flag for coroutine support
add_compile_options(-fcoroutines)


# Add executable
add_executable(myprogram ${SOURCES})

# Link with Boost libraries
target_link_libraries(myprogram ${Boost_LIBRARIES})
