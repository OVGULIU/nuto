add_unit_test(Average)
add_unit_test(LinearInterpolation math/Interpolation.cpp)
add_unit_test(CubicSplineInterpolation math/Interpolation.cpp)
add_unit_test(EigenCompanion)
add_unit_test(EigenIO)

add_unit_test(EigenSparseSolve base/Logger.cpp base/Timer.cpp)
if(SUITESPARSE_FOUND)
    target_link_libraries(EigenSparseSolve SuiteSparse::UmfPack SuiteSparse::Cholmod)
endif()
if(MUMPS_FOUND)
    target_link_libraries(EigenSparseSolve EigenMumpsSupport)
endif()

add_unit_test(Legendre)
add_unit_test(NaturalCoordinateMemoizer)
add_unit_test(NewtonRaphson
    math/EigenSparseSolve.cpp
    base/Logger.cpp
    base/Timer.cpp
    )

add_unit_test(Gmres)
add_unit_test(Shapes
    mechanics/interpolation/InterpolationTrussLinear.cpp
    mechanics/interpolation/InterpolationTriangleLinear.cpp
    mechanics/interpolation/InterpolationTetrahedronLinear.cpp
    mechanics/interpolation/InterpolationPrismLinear.cpp
    )
