cmake_minimum_required(VERSION 3.10.0)
project(name-not-found-404)

# enable coverage flags
option(ENABLE_COVERAGE "Enable coverage flags" OFF)
if(ENABLE_COVERAGE)
  add_compile_options(-O0 -g --coverage)
  link_libraries(--coverage)
endif()

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system filesystem log log_setup regex)
message(STATUS "Boost version: ${Boost_VERSION}")

# Enable nlohmann_json
find_package(nlohmann_json 3.2.0 REQUIRED)
message(STATUS "nlohmann_json version: ${nlohmann_json_VERSION}")

include_directories(include)

# libraries go under here
add_library(server_main_lib src/server_main.cc)
add_library(server_lib src/server.cc)
add_library(handler_registry src/handler_registry.cc)
add_library(session_lib src/session.cc)
add_library(config_parser_lib src/config_parser.cc)
add_library(request_parser_lib src/request_parser.cc)
add_library(logger_lib src/logger.cc)
add_library(dispatcher_lib src/dispatcher.cc)
add_library(disk_file_store_lib src/disk_file_store.cc)
add_library(fake_file_store_lib src/fake_file_store.cc)

#handler libraries
add_library(echo_handler_lib OBJECT src/echo_handler.cc)
add_library (not_found_handler_lib OBJECT src/not_found_handler.cc)
add_library(static_handler_lib OBJECT src/static_handler.cc)
add_library(api_handler_lib OBJECT src/api_handler.cc)

#link dispatcher
target_link_libraries(config_parser_lib dispatcher_lib)
target_link_libraries(session_lib dispatcher_lib)

target_link_libraries(echo_handler_lib
  PUBLIC handler_registry
)
target_link_libraries(static_handler_lib
  PUBLIC handler_registry
)
target_link_libraries(not_found_handler_lib
  PUBLIC handler_registry
)
target_link_libraries(disk_file_store_lib
  PUBLIC Boost::filesystem
)
target_link_libraries(fake_file_store_lib
  PUBLIC Boost::filesystem
)
target_link_libraries(api_handler_lib PUBLIC handler_registry 
  disk_file_store_lib fake_file_store_lib nlohmann_json::nlohmann_json
)

# executables go under here
add_executable(server src/server_main.cc)
target_link_libraries(server logger_lib server_main_lib server_lib 
session_lib config_parser_lib echo_handler_lib not_found_handler_lib 
static_handler_lib request_parser_lib Boost::log_setup Boost::log 
Boost::system Boost::regex api_handler_lib disk_file_store_lib 
fake_file_store_lib nlohmann_json::nlohmann_json)

add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test config_parser_lib 
  logger_lib
  echo_handler_lib
  not_found_handler_lib
  static_handler_lib
  Boost::log
  Boost::log_setup
  Boost::system
  gtest_main)

add_executable(not_found_handler_test tests/not_found_handler_test.cc)
target_link_libraries(not_found_handler_test not_found_handler_lib gtest_main gmock_main)

add_executable(echo_handler_test tests/echo_handler_test.cc)
target_link_libraries(echo_handler_test echo_handler_lib request_parser_lib gtest_main gmock_main)

add_executable(static_handler_test tests/static_handler_test.cc)
target_link_libraries(static_handler_test static_handler_lib request_parser_lib gtest_main)

add_executable(request_parser_test tests/request_parser_test.cc)
target_link_libraries(request_parser_test request_parser_lib gtest_main)

add_executable(api_handler_test tests/api_handler_test.cc)
target_link_libraries(api_handler_test api_handler_lib fake_file_store_lib 
  request_parser_lib gtest_main Boost::system)

add_executable(dispatcher_test tests/dispatcher_test.cc)
target_link_libraries(dispatcher_test dispatcher_lib gtest_main gmock_main)

add_executable(parse_common_api_test tests/parse_common_api_test.cc)
target_link_libraries(parse_common_api_test 
  config_parser_lib 
  echo_handler_lib
  not_found_handler_lib
  static_handler_lib
  Boost::log
  Boost::log_setup
  Boost::system
  gtest_main)

add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test
  server_lib
  session_lib
  request_parser_lib
  Boost::system
  Boost::log_setup
  Boost::log
  gtest_main)

add_executable(logger_test tests/logger_test.cc)
target_link_libraries(logger_test
  logger_lib
  Boost::log
  Boost::log_setup
  Boost::system
  gtest_main)


# tests go under here
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(echo_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(not_found_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(static_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(request_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(dispatcher_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(logger_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(api_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Update with target/test targets
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(
  TARGETS 
    server
    server_main_lib
    server_lib 
    session_lib 
    config_parser_lib 
    echo_handler_lib 
    not_found_handler_lib 
    static_handler_lib
    logger_lib
    dispatcher_lib
    api_handler_lib
  TESTS 
    config_parser_test
    echo_handler_test
    static_handler_test
    not_found_handler_test
    request_parser_test
    server_test
    logger_test
    parse_common_api_test
    dispatcher_test
    api_handler_test
  )

# Bash integration test for the echo server
add_test(NAME EchoServerIntegrationTest
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_test.sh)

# Bash integration test for the echo server
add_test(NAME StaticHandlerIntegrationTest
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/static_integration_test.sh)

add_test(NAME NotFoundHandlerIntegrationTest
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/not_found_integration_test.sh)