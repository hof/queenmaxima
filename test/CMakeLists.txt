
find_package(Boost 1.49 COMPONENTS system unit_test_framework REQUIRED) 

include_directories(${maxima_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})

add_executable(fenton fenton.cpp)

message(STATUS "Boost Libraries for test: ${Boost_LIBRARIES}")

target_link_libraries(fenton ${Boost_LIBRARIES} maxchess)
