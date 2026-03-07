cmake_minimum_required(VERSION 3.16)
project(llm_agent_examples CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(CURL REQUIRED)

add_executable(calculator examples/calculator.cpp)
target_include_directories(calculator PRIVATE include)
target_link_libraries(calculator CURL::libcurl)

add_executable(web_search_mock examples/web_search_mock.cpp)
target_include_directories(web_search_mock PRIVATE include)
target_link_libraries(web_search_mock CURL::libcurl)
