cmake_minimum_required(VERSION 3.0)


project(disney)


if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed.  Change directory to a different location and build again.")
endif()


add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

add_definitions(-DGLFW_DLL)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MSVC)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()


set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


find_package(CURL CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")


add_subdirectory(app)
