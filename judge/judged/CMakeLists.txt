CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(judged C)
SET(CMAKE_BUILD_TYPE Debug)

SET(CMAKE_C_STANDARD 90)
SET(CMAKE_C_FLAGS "-Wall -std=c90 -march=native -O3")
SET(CMAKE_C_FLAGS_DEBUG "-g -Wall")

# include
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
# source
FILE(GLOB SOURCE "src/*.c")

# make dynamic linked libirary
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR})
ADD_LIBRARY(judge SHARED ${SOURCE})
TARGET_LINK_LIBRARIES(judge -lpthread -lseccomp)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# make daemon binary
#LINK_DIRECTORIES(${PROJECT_SOURCE_DIR})
#ADD_EXECUTABLE(judged ${SOURCE})
#TARGET_LINK_LIBRARIES(judged -lpthread -lseccomp)
#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
