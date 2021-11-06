add_library (wfcommon executor.cc tile.cc stripe.cc) 

add_executable (praster main.cc) 

if(CMAKE_BUILD_TYPE MATCHES Release)
add_definitions (-DGSL_RANGE_CHECK_OFF -DHAVE_INLINE)
ENDIF()

target_include_directories(wfcommon
    PRIVATE
    ${GSL_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    PUBLIC
    ../include
)

target_link_libraries(wfcommon
    PRIVATE
    ${GSL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
)

target_include_directories(praster
    PRIVATE
    ${GDAL_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    PUBLIC
    ../include
)

target_link_libraries(praster
    PRIVATE
    ${GDAL_LIBRARIES}
    ${Boost_LIBRARIES}
    wfcommon
)