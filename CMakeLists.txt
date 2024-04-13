cmake_minimum_required(VERSION 3.15)

project(sfs)

option(USE_SYSTEM_GRPC "Use system installed gRPC" ON)


if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

find_package(Threads REQUIRED)

if(USE_SYSTEM_GRPC)
  # Find Protobuf installation
  # Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
  option(protobuf_MODULE_COMPATIBLE TRUE)
  find_package(Protobuf CONFIG REQUIRED)
  message(STATUS "Using protobuf ${Protobuf_VERSION}")

  set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
  set(_REFLECTION gRPC::grpc++_reflection)
  if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
  else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
  endif()

  # Find gRPC installation
  # Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
  find_package(gRPC CONFIG REQUIRED)
  message(STATUS "Using gRPC ${gRPC_VERSION}")

  set(_GRPC_GRPCPP gRPC::grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
  endif()
else()
  include(FetchContent)
  set(ABSL_ENABLE_INSTALL ON)
  FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc
    GIT_TAG        v1.62.1
  )
  set(FETCHCONTENT_QUIET OFF)
  FetchContent_MakeAvailable(gRPC)

  # Since FetchContent uses add_subdirectory under the hood, we can use
  # the grpc targets directly from this build.
  set(_PROTOBUF_LIBPROTOBUF libprotobuf)
  set(_REFLECTION grpc++_reflection)
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
  set(_GRPC_GRPCPP grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
  endif()
endif()

# Proto file
get_filename_component(sfs_proto "protos/sfs.proto" ABSOLUTE)
get_filename_component(sfs_proto_path "${sfs_proto}" PATH)

# Generated sources
set(sfs_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/sfs.pb.cc")
set(sfs_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/sfs.pb.h")
set(sfs_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/sfs.grpc.pb.cc")
set(sfs_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/sfs.grpc.pb.h")
add_custom_command(
      OUTPUT "${sfs_proto_srcs}" "${sfs_proto_hdrs}" "${sfs_grpc_srcs}" "${sfs_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${sfs_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${sfs_proto}"
      DEPENDS "${sfs_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# hw_grpc_proto
add_library(sfs_grpc_proto
  ${sfs_grpc_srcs}
  ${sfs_grpc_hdrs}
  ${sfs_proto_srcs}
  ${sfs_proto_hdrs})
target_link_libraries(sfs_grpc_proto
  absl::check
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

foreach(_target
  sfs_client sfs_server)
  add_executable(${_target} "${_target}.cc")
  target_link_libraries(${_target}
    sfs_grpc_proto
    absl::check
    absl::flags
    absl::flags_parse
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()
