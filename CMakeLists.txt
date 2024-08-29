cmake_minimum_required(VERSION 3.14)
project(easer-lib VERSION 1.0.0 DESCRIPTION "A easy to use streaming and serialization library" LANGUAGES CXX)

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

set(LIB_SOURCES
)

set(LIB_HEADERS 
	"${CMAKE_SOURCE_DIR}/include/easer/base.h"
	"${CMAKE_SOURCE_DIR}/include/easer/streamer.h"
	"${CMAKE_SOURCE_DIR}/include/easer/easer.h"
)

add_library(${PROJECT_NAME} ${LIB_HEADERS} ${LIB_SOURCES})
target_include_directories(${PROJECT_NAME}
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${Boost_INCLUDE_DIRS}
)
set_target_properties(${PROJECT_NAME} PROPERTIES
	LINKER_LANGUAGE CXX
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

# Testing
set(TEST_SOURCES
	"${CMAKE_SOURCE_DIR}/test/tester.cpp"
)

set(TEST_NAME ${PROJECT_NAME}-tester)
add_executable(${TEST_NAME} ${TEST_SOURCES})
set_target_properties(${TEST_NAME} PROPERTIES
	LINKER_LANGUAGE CXX
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)
target_link_libraries(${TEST_NAME} 
	PRIVATE
	${PROJECT_NAME}
	Boost::filesystem
	Boost::system
	Boost::unit_test_framework
)
add_test(
	NAME ${TEST_NAME}
	COMMAND ${TEST_NAME}
)

