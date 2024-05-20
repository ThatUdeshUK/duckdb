cmake_minimum_required(VERSION 3.5)

project(duckdb_torch CXX)
if(POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "${LIBTORCH_PATH}/share/cmake/Torch")

find_package(Torch REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

include_directories( ${LIBTORCH_INCLUDE_DIRS} )

add_library(duckdb_torch STATIC duckdb_torch.cpp)

target_link_libraries(duckdb_torch PUBLIC ${TORCH_LIBRARIES})

set_target_properties(duckdb_torch PROPERTIES EXPORT_NAME duckdb_torch)

install(TARGETS duckdb_torch
        EXPORT "${DUCKDB_EXPORT_SET}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")

disable_target_warnings(duckdb_torch)
