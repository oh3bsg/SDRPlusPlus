cmake_minimum_required(VERSION 3.13)
project(search_scan)

file(GLOB SRC "src/*.cpp")

add_library(search_scan SHARED ${SRC})
target_link_libraries(search_scan PRIVATE sdrpp_core)
set_target_properties(search_scan PROPERTIES PREFIX "")

target_include_directories(search_scan PRIVATE "src/")

if (MSVC)
    target_compile_options(search_scan PRIVATE /O2 /Ob2 /std:c++17 /EHsc)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(search_scan PRIVATE -O3 -std=c++17 -Wno-unused-command-line-argument -undefined dynamic_lookup)
else ()
    target_compile_options(search_scan PRIVATE -O3 -std=c++17)
endif ()

# Install directives
install(TARGETS search_scan DESTINATION lib/sdrpp/plugins)