unset(all_integration_tests CACHE)

add_subdirectory(geometryConcrete)
add_subdirectory(math)
add_subdirectory(mechanics)
add_subdirectory(visualize)

add_custom_target(integrationtests DEPENDS ${all_integration_tests})
