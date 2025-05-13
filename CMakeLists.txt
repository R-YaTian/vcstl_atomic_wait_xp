cmake_minimum_required(VERSION 3.15)
project(msvcp_atomic_wait LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

get_filename_component(TOOLSET_BINARIES_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY) # Example: $\VC\Tools\MSVC\14.23.27931\bin\Hostx86\x86
get_filename_component(TOOLSET_ROOT_DIR "${TOOLSET_BINARIES_DIR}" DIRECTORY) # $\VC\Tools\MSVC\14.23.27931\bin\Hostx86
get_filename_component(TOOLSET_ROOT_DIR "${TOOLSET_ROOT_DIR}" DIRECTORY) # $\VC\Tools\MSVC\14.23.27931\bin
get_filename_component(TOOLSET_ROOT_DIR "${TOOLSET_ROOT_DIR}" DIRECTORY) # $\VC\Tools\MSVC\14.23.27931
include_directories(BEFORE
    "${TOOLSET_ROOT_DIR}/crt/src/vcruntime"
)

# set pch config
set(PCH_HEADER "pch.h")
set(PCH_SOURCE "pch.cpp")

set(VCLIBS_SUFFIX "_oss" CACHE STRING "suffix for built DLL names to avoid conflicts with distributed DLLs")

function(generate_satellite_def SATELLITE_NAME D_SUFFIX)
    set(full_satellite_name "msvcp140${D_SUFFIX}_${SATELLITE_NAME}${VCLIBS_SUFFIX}")
    string(TOUPPER "${full_satellite_name}" upper_full_satellite_name)
    set(satellite_input_src_file_path "${CMAKE_CURRENT_LIST_DIR}/msvcp_${SATELLITE_NAME}.src")
    set(satellite_output_def_file_path "${CMAKE_CURRENT_LIST_DIR}/msvcp_${SATELLITE_NAME}${D_SUFFIX}.def")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${satellite_input_src_file_path}")

    # We use the placeholder name "LIBRARYNAME" in the corresponding .src file of a satellite DLL
    # (i.e., we write "LIBRARY LIBRARYNAME" as the first non-commented line in the file).
    #
    # Here, we dynamically replace this placeholder name with the name of the satellite DLL for
    # the current build configuration. Then, we write out the new .def file to the binary output
    # directory.
    file(READ "${satellite_input_src_file_path}" satellite_def_file_contents)
    string(REPLACE "LIBRARYNAME" "${upper_full_satellite_name}" satellite_def_file_contents "${satellite_def_file_contents}")
    file(GENERATE OUTPUT "${satellite_output_def_file_path}" CONTENT "${satellite_def_file_contents}")
endfunction()

generate_satellite_def("atomic_wait" "${D_SUFFIX}")

# Add sources
set(COMMON_SOURCES
    ${PCH_SOURCE}
    atomic_wait.cpp
    parallel_algorithms.cpp
    syncstream.cpp
    tzdb.cpp
)

set(DLL_SOURCES
    ${COMMON_SOURCES}
    dllmain.cpp
)

# Add headers
set(HEADERS
    init_locks.hpp
    framework.h
    ${PCH_HEADER}
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SUBSYSTEM_VERSION "5.02")
else()
    set(SUBSYSTEM_VERSION "5.01")
endif()

# Building dll
add_library(msvcp_atomic_wait SHARED ${DLL_SOURCES} ${HEADERS})
target_link_options(msvcp_atomic_wait PRIVATE "/SUBSYSTEM:CONSOLE,${SUBSYSTEM_VERSION}")
target_precompile_headers(msvcp_atomic_wait PRIVATE ${PCH_HEADER})

# define export symbols
set_target_properties(msvcp_atomic_wait PROPERTIES LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/msvcp_atomic_wait.def")
set_target_properties(msvcp_atomic_wait PROPERTIES
  LINK_FLAGS "/ENTRY:DllMainCRTStartupForYY_Thunks"
)

# DLL target
target_compile_definitions(msvcp_atomic_wait PRIVATE
    _WINDLL
    UNICODE
    _UNICODE
)

# Building static lib
add_library(msvcp_atomic_wait_static STATIC ${COMMON_SOURCES} ${HEADERS})
target_precompile_headers(msvcp_atomic_wait_static PRIVATE ${PCH_HEADER})

# Static target
target_compile_definitions(msvcp_atomic_wait_static PRIVATE
    UNICODE
    _UNICODE
)
