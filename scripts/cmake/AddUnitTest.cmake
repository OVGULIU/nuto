# `add_unit_test(SomeClass)` builds the unit test of the name `SomeClass`,
# links it to the required libraries and adds it to the test suite under an
# appropriate name
function(add_unit_test ClassName)
    # find relative path, i.e. remove the `.../test/`
    string(REPLACE "${CMAKE_SOURCE_DIR}/test/" ""
        relpath ${CMAKE_CURRENT_SOURCE_DIR})

    if(ARGN)
        foreach(filename ${ARGN})
            set(AdditionalSources "${AdditionalSources};${CMAKE_SOURCE_DIR}/nuto/${filename}")
        endforeach()
    endif()

    set(CorrespondingCpp "${CMAKE_SOURCE_DIR}/nuto/${relpath}/${ClassName}.cpp")
    if(EXISTS ${CorrespondingCpp})
        add_executable(${ClassName} ${ClassName}.cpp ${CorrespondingCpp} ${AdditionalSources})
    else()
        add_executable(${ClassName} ${ClassName}.cpp ${AdditionalSources})
    endif()
    # link the unit test framework to the unit test
    target_link_libraries(${ClassName} Boost::unit_test_framework)
    target_include_directories(${ClassName} PRIVATE ${CMAKE_SOURCE_DIR})
    target_include_directories(${ClassName}
        PRIVATE ${CMAKE_SOURCE_DIR}/test/tools)
    target_link_libraries(${ClassName} Fakeit Eigen3::Eigen)

    # generate a ctest name for the test
    string(REPLACE "/" "::" testname ${relpath})
    add_test(unit::${testname}::${ClassName} ${ClassName} --log_level=message)
    set(all_unit_tests "${all_unit_tests};${ClassName}"
        CACHE INTERNAL "The names of all the unit tests")
endfunction()
