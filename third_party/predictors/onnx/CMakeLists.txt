cmake_minimum_required(VERSION 3.5)

project(duckdb_onnx CXX)
if(POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_library(ONNX_RUNTIME_LIB onnxruntime HINTS /usr/local/lib)
message(STATUS "ORT Include Libs: ${ONNX_RUNTIME_SESSION_INCLUDE_DIRS}")

include_directories( ${LIBTORCH_INCLUDE_DIRS} )

add_library(duckdb_onnx STATIC duckdb_onnx.cpp)

target_link_libraries(duckdb_onnx PUBLIC ${ONNX_RUNTIME_LIB})

target_include_directories(duckdb_onnx PUBLIC ${ONNX_RUNTIME_SESSION_INCLUDE_DIRS})

set_target_properties(duckdb_onnx PROPERTIES EXPORT_NAME duckdb_onnx)

install(TARGETS duckdb_onnx
        EXPORT "${DUCKDB_EXPORT_SET}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")

disable_target_warnings(duckdb_onnx)
