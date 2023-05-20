cmake_minimum_required(VERSION 3.15)

project(d_gen)

set(CMAKE_CXX_STANDARD 17)

add_executable(d_gen main.cpp antlr_test.cpp z3_test.cpp)
# rdynamic, needed to discover functions from d_gen by llvm
set_property(TARGET d_gen PROPERTY ENABLE_EXPORTS ON)

#ANTLR
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
add_definitions(-DANTLR4CPP_STATIC)
# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)
set(ANTLR4_ZIP_REPOSITORY https://github.com/antlr/antlr4/archive/refs/tags/4.12.0.zip)
# add external build for antlrcpp
include(ExternalAntlr4Cpp)
# add antrl4cpp artifacts to project environment
message(STATUS "antlr include dirs ${ANTLR4_INCLUDE_DIRS}")
target_include_directories(d_gen PRIVATE ${ANTLR4_INCLUDE_DIRS})
set(ANTLR_EXECUTABLE /usr/local/lib/antlr-4.12.0-complete.jar)

find_package(ANTLR REQUIRED)
message(STATUS "ANTLR_FOUND: ${ANTLR_FOUND}")
antlr_target(Dgen d_gen.g4 VISITOR LISTENER)

target_include_directories(d_gen PRIVATE ${ANTLR_Dgen_OUTPUT_DIR})
target_link_libraries(d_gen PRIVATE antlr4_static)

message(STATUS "antlr libs ${ANTLR_Dgen_OUTPUT_DIR} ${ANTLR_Dgen_CXX_OUTPUTS}}")
target_sources(d_gen PRIVATE ${ANTLR_Dgen_CXX_OUTPUTS})
#ANTLR

#Z3
#find_package(Z3 REQUIRED CONFIG)
#message(STATUS "Z3_FOUND: ${Z3_FOUND}")
#message(STATUS "Found Z3 ${Z3_VERSION_STRING}")
#message(STATUS "Z3_DIR: ${Z3_DIR}")
#
#target_include_directories(d_gen PRIVATE ${Z3_CXX_INCLUDE_DIRS})
#target_link_libraries(d_gen PRIVATE ${Z3_LIBRARIES})
#
#target_compile_options(d_gen PRIVATE ${Z3_COMPONENT_CXX_FLAGS})
#
#if (CMAKE_SYSTEM_NAME MATCHES "[Ww]indows")
#	# On Windows we need to copy the Z3 libraries
#	# into the same directory as the executable
#	# so that they can be found.
#	foreach (z3_lib ${Z3_LIBRARIES})
#		message(STATUS "Adding copy rule for ${z3_lib}")
#		add_custom_command(TARGET d_gen
#				POST_BUILD
#				COMMAND
#				${CMAKE_COMMAND} -E copy_if_different
#				$<TARGET_FILE:${z3_lib}>
#				$<TARGET_FILE_DIR:d_gen>
#				)
#	endforeach()
#endif()
#Z3

#LLVM
find_package(LLVM 13 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

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
)

message(STATUS "llvm libs: ${llvm_libs}")
message(STATUS "llvm include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "llvm defs: ${LLVM_DEFINITIONS}")
message(STATUS "llvm rtti: ${LLVM_ENABLE_RTTI}")

target_include_directories(d_gen PRIVATE ${LLVM_INCLUDE_DIRS})
# workaround for clion
include_directories(/usr/include/llvm-13)

target_link_libraries(d_gen PRIVATE ${llvm_libs})

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
#LLVM
