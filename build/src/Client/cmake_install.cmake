# Install script for directory: /home/ve/Downloads/pybo/src/Client

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/ve/Downloads/pybo/bin")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if(EXISTS "$ENV{DESTDIR}/home/ve/Downloads/pybo/bin/pseuwow" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ve/Downloads/pybo/bin/pseuwow")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/ve/Downloads/pybo/bin/pseuwow"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ve/Downloads/pybo/bin/pseuwow")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/ve/Downloads/pybo/bin" TYPE EXECUTABLE FILES "/home/ve/Downloads/pybo/build/src/Client/pseuwow")
  if(EXISTS "$ENV{DESTDIR}/home/ve/Downloads/pybo/bin/pseuwow" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ve/Downloads/pybo/bin/pseuwow")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/home/ve/Downloads/pybo/bin/pseuwow"
         OLD_RPATH "/home/ve/Downloads/pybo/bin:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/ve/Downloads/pybo/bin/pseuwow")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/ve/Downloads/pybo/build/src/Client/DefScript/cmake_install.cmake")

endif()

