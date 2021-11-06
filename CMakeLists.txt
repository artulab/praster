cmake_minimum_required (VERSION 3.16)

project(
    praster 
    VERSION 0.1
    DESCRIPTION "A C++ library to process raster data in parallel for shared-memory systems"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    enable_testing()

    add_subdirectory (test)

    # use -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)
  
    # support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  
    include(CTest)
  
    # Docs only available if this is the main app
    find_package(Doxygen)
    if(Doxygen_FOUND)
      add_subdirectory(docs)
    else()
      message(STATUS "Doxygen not found, not building docs")
    endif()
endif()

find_package(Threads REQUIRED)

set(Boost_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.67 REQUIRED COMPONENTS 
    thread 
    chrono
    program_options)

find_package(GDAL REQUIRED)

if(NOT GDAL_FOUND)
    message(FATAL_ERROR "GDAL not found, cannot proceed")
endif()

find_package(GSL REQUIRED)

if(NOT GSL_FOUND)
    message(FATAL_ERROR "GSL not found, cannot proceed")
endif()

add_subdirectory (src)