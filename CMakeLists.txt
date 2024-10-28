cmake_minimum_required(VERSION 3.14)
project(easer-lib VERSION 1.0.0 DESCRIPTION "A easy to use streaming and serialization library" LANGUAGES CXX)

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

set(LIB_SOURCES
)

set(LIB_HEADERS 
	"${CMAKE_SOURCE_DIR}/include/easer/easer.h"
)

add_library(${PROJECT_NAME} ${LIB_HEADERS} ${LIB_SOURCES})
target_include_directories(${PROJECT_NAME}
	PUBLIC
	    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
		${Boost_INCLUDE_DIRS}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	LINKER_LANGUAGE CXX
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

# Installation
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
	EXPORT ${PROJECT_NAME}-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE easer::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
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

