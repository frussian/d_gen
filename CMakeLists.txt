cmake_minimum_required(VERSION 3.24)
project(genter)

set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

add_executable(genter main.cpp)

llvm_map_components_to_libnames(
		llvm_libs
		Analysis
		Core
		ExecutionEngine
		InstCombine
		Object
		OrcJIT
		RuntimeDyld
		ScalarOpts
		Support
		TransformUtils
		native
		irreader
)

message(STATUS "llvm libs: ${llvm_libs}")
message(STATUS "llvm include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "llvm defs: ${LLVM_DEFINITIONS}")
message(STATUS "llvm rtti: ${LLVM_ENABLE_RTTI}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Link against LLVM libraries
target_link_libraries(genter ${llvm_libs})
