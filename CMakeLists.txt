cmake_minimum_required (VERSION 2.6)
project (Pybo)


# Inspired by MaNGOS

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_SOURCE_DIR}/cmake
)

# Force out-of-source build
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" BUILDING_IN_SOURCE)
if(BUILDING_IN_SOURCE)
  message(FATAL_ERROR
    "This project requires an out of source build. Remove the file 'CMakeCache.txt' found in this directory before continuing, create a separate build directory and run
 'cmake <srcs> [options]' from there."
  )
endif()

#option(DEBUG "Debug mode" 1)
#option(BUILD_TOOLS "Build Tools" 0)
set(DEBUG 1)


find_package(Platform REQUIRED)
find_package(OpenSSL REQUIRED)



# 
if(DEBUG)
  message("Build in debug-mode   : Yes")
  set(CMAKE_BUILD_TYPE Debug)
else()
  set(CMAKE_BUILD_TYPE Release)
  message("Build in debug-mode   : No  (default)")
endif()
# Handle debugmode compiles (this will require further work for proper WIN32-setups)
if(UNIX)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()

# Set warning levels for different builds
if(UNIX)
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --no-warnings")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --no-warnings")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wfatal-errors -Wextra")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wfatal-errors -Wextra")
endif()


# Set up the install-prefix
if(CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
  get_filename_component(PREFIX_ABSOLUTE "./bin" ABSOLUTE)
  set(CMAKE_INSTALL_PREFIX ${PREFIX_ABSOLUTE} CACHE PATH "Install path prefix." FORCE)
endif()





message("")
message("Install binaries to     : ${CMAKE_INSTALL_PREFIX}")
message("")

add_subdirectory (src)
