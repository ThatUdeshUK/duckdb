cmake_minimum_required(VERSION 3.5)

project(duckdb_torch CXX)
if(POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)

#message(STATUS "Source ->${LIBTORCH_INCLUDE_DIRS}")
#message(STATUS "Target ->${CMAKE_CURRENT_SOURCE_DIR}/include/libtorch")
#execute_process(COMMAND "ln -s ${LIBTORCH_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include/libtorch")

set(CMAKE_PREFIX_PATH "${LIBTORCH_PATH}/share/cmake/Torch")

find_package(Torch REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

include_directories( ${LIBTORCH_INCLUDE_DIRS} )

add_library(duckdb_torch STATIC duckdb_torch.cpp)

target_link_libraries(duckdb_torch PUBLIC ${TORCH_LIBRARIES})

if(BUILD_PREDICT)
    message(STATUS "BUILD_PREDICT set!!! - duckdb_torch")
    #  include(${CMAKE_CURRENT_SOURCE_DIR}/tools/pythonpkg/duckdb_extension_config.cmake)
endif()

target_include_directories(
        duckdb_torch
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

set_target_properties(duckdb_torch PROPERTIES EXPORT_NAME duckdb_torch)

install(TARGETS duckdb_torch
        EXPORT "${DUCKDB_EXPORT_SET}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")

disable_target_warnings(duckdb_torch)
