# Install script for directory: /root/muduo/03/jmuduo/muduo/base

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/root/muduo/03/build/debug-install")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "debug")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/root/muduo/03/build/debug/lib/libmuduo_base.a")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/muduo/base" TYPE FILE FILES
    "/root/muduo/03/jmuduo/muduo/base/Timestamp.h"
    "/root/muduo/03/jmuduo/muduo/base/copyable.h"
    "/root/muduo/03/jmuduo/muduo/base/Types.h"
    "/root/muduo/03/jmuduo/muduo/base/Atomic.h"
    "/root/muduo/03/jmuduo/muduo/base/Exception.h"
    "/root/muduo/03/jmuduo/muduo/base/ThreadLocal.h"
    "/root/muduo/03/jmuduo/muduo/base/ThreadLocalSingleton.h"
    "/root/muduo/03/jmuduo/muduo/base/ThreadPool.h"
    "/root/muduo/03/jmuduo/muduo/base/CurrentThread.h"
    "/root/muduo/03/jmuduo/muduo/base/Mutex.h"
    "/root/muduo/03/jmuduo/muduo/base/FileUtil.h"
    "/root/muduo/03/jmuduo/muduo/base/BoundedBlockingQueue.h"
    "/root/muduo/03/jmuduo/muduo/base/Thread.h"
    "/root/muduo/03/jmuduo/muduo/base/Condition.h"
    "/root/muduo/03/jmuduo/muduo/base/CountDownLatch.h"
    "/root/muduo/03/jmuduo/muduo/base/Logging.h"
    "/root/muduo/03/jmuduo/muduo/base/LogStream.h"
    "/root/muduo/03/jmuduo/muduo/base/StringPiece.h"
    "/root/muduo/03/jmuduo/muduo/base/ProcessInfo.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/root/muduo/03/build/debug/muduo/base/tests/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

