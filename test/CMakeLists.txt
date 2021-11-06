set (tests test_tile test_stripe)

foreach (test ${tests})
    add_executable(${test} ${test}.cc)

    add_test(NAME ${test} COMMAND ${test})

    target_include_directories(${test}
        PRIVATE
        ${GDAL_INCLUDE_DIRS}
        ${GSL_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ../include
    )

    target_link_libraries(${test}
        PRIVATE
        ${GDAL_LIBRARIES}
        ${GSL_LIBRARIES}
        ${Boost_LIBRARIES}
        wfcommon
    )
endforeach (test ${tests})