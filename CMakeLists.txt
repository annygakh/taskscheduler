cmake_minimum_required(VERSION 3.7)
project(taskscheduler)

set(CMAKE_CXX_STANDARD 11)

FIND_PACKAGE( Boost 1.64 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

set(SOURCE_FILES main.cpp TaskScheduler.h Task.h Database.h Metric.h Log.h)
add_executable(taskscheduler ${SOURCE_FILES})
target_link_libraries(taskscheduler LINK_PUBLIC sqlite3)
target_link_libraries(taskscheduler LINK_PUBLIC ${Boost_LIBRARIES})

