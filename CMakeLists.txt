cmake_minimum_required(VERSION 3.15)

project(d_gen
		VERSION 1.0.0
		DESCRIPTION "Test Data Generator"
		LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT DEFINED D_GEN_INSTALL_CMAKEDIR)
	set(D_GEN_INSTALL_CMAKEDIR
			"${CMAKE_INSTALL_LIBDIR}/cmake/d_gen-${PROJECT_VERSION}" CACHE STRING
			"Install path for d_gen package-related CMake files")
endif()


add_library(d_gen SHARED) # initialized below
add_library(d_gen::d_gen ALIAS d_gen)

set(public_headers
		include/d_gen/DGen.h
		include/d_gen/BuildError.h
		include/d_gen/Position.h)

set(sources
		src/z3_test.cpp src/ast.h
		src/ast.cpp src/utils/assert.h src/type.h
		src/type.cpp src/ASTBuilderVisitor.cpp src/ASTBuilderVisitor.h
		src/BuildError.cpp src/Semantics.cpp
		src/Semantics.h src/Symbol.cpp src/Symbol.h
		src/SymbolTable.cpp src/SymbolTable.h src/Position.cpp
		src/CodegenVisitor.cpp src/CodegenVisitor.h
		src/DGenJIT.cpp src/DGenJIT.h src/LLVMCtx.h src/DGen.cpp
		src/CodegenZ3Visitor.cpp src/CodegenZ3Visitor.h
		${public_headers})

target_sources(d_gen PRIVATE ${sources})

target_include_directories(d_gen
		PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/d_gen>"
		PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/src")

set_target_properties(d_gen PROPERTIES
		PUBLIC_HEADER "${public_headers}"
		SOVERSION ${PROJECT_VERSION_MAJOR}
		VERSION ${PROJECT_VERSION})


#ANTLR
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/cmake)
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
find_package(Z3 REQUIRED CONFIG)
message(STATUS "Z3_FOUND: ${Z3_FOUND}")
message(STATUS "Found Z3 ${Z3_VERSION_STRING}")
message(STATUS "Z3_DIR: ${Z3_DIR}")

target_include_directories(d_gen PRIVATE ${Z3_CXX_INCLUDE_DIRS})
target_link_libraries(d_gen PRIVATE ${Z3_LIBRARIES})

target_compile_options(d_gen PRIVATE ${Z3_COMPONENT_CXX_FLAGS})

if (CMAKE_SYSTEM_NAME MATCHES "[Ww]indows")
	# On Windows we need to copy the Z3 libraries
	# into the same directory as the executable
	# so that they can be found.
	foreach (z3_lib ${Z3_LIBRARIES})
		message(STATUS "Adding copy rule for ${z3_lib}")
		add_custom_command(TARGET d_gen
				POST_BUILD
				COMMAND
				${CMAKE_COMMAND} -E copy_if_different
				$<TARGET_FILE:${z3_lib}>
				$<TARGET_FILE_DIR:d_gen>
				)
	endforeach()
endif()
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

configure_package_config_file(cmake/d_gen-config.cmake.in d_gen-config.cmake
		INSTALL_DESTINATION "${D_GEN_INSTALL_CMAKEDIR}")

write_basic_package_version_file(d_gen-config-version.cmake
		COMPATIBILITY SameMajorVersion)

install(TARGETS d_gen EXPORT d_gen_export
		RUNTIME COMPONENT d_gen
		LIBRARY COMPONENT d_gen NAMELINK_COMPONENT d_gen-dev
		ARCHIVE COMPONENT d_gen-dev
		PUBLIC_HEADER COMPONENT d_gen-dev
		DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/d_gen")

set(targets_file "d_gen-targets.cmake")

install(EXPORT d_gen_export
		COMPONENT d_gen-dev
		FILE "${targets_file}"
		DESTINATION "${D_GEN_INSTALL_CMAKEDIR}"
		NAMESPACE d_gen::)

install(FILES
		"${CMAKE_CURRENT_BINARY_DIR}/d_gen-config.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/d_gen-config-version.cmake"
		COMPONENT d_gen-dev
		DESTINATION "${D_GEN_INSTALL_CMAKEDIR}")
