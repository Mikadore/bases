cmake_minimum_required(VERSION 3.11) # idk what to put here

project(BaseConv)

set(CMAKE_CXX_STANDARD 20)

# testing purposes only
# add_executable(baseconv main.cpp)
# find_package(fmt)
# target_link_libraries(baseconv PRIVATE fmt::fmt)

add_executable(tests tests/test.cpp tests/base64.cpp tests/generated/base64-fuzz.cpp)
target_include_directories(tests PRIVATE include)

