# Minimum CMake required. If available, accept the policy-controlled behavior up
# to 3.26.
cmake_minimum_required(VERSION 3.16...3.26)

# Revert to old behavior for MSVC debug symbols.
if(POLICY CMP0141)
  cmake_policy(SET CMP0141 OLD)
endif()

project(SplitterYolo
	VERSION 0.0.1
	LANGUAGES C CXX
	)

Set(SplitterYolo_VERSION "1.0.0")

set (CMAKE_CXX_STANDARD 17)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wfatal-errors")

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEFAULT_LOG_LEVEL=4")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEFAULT_LOG_LEVEL=1")

message(STATUS "ODOO Test version: ${SplitterYolo_VERSION}")

add_executable(SplitterYolo checkARRAY.cpp  main.cpp)
