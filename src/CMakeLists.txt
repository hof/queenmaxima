
find_package(Boost 1.49 COMPONENTS system REQUIRED) 
include_directories(${Boost_INCLUDE_DIRS})

add_library(maxchess 
	attack.cpp 
	checkcheck.cpp
    extensions.cpp
    hash.cpp
    knightscore.cpp
    parser.cpp
    run_test.cpp
    w0_evaluate.cpp
    w0_search_b.cpp
    w17_search.cpp
    bbtables.cpp
    db_pg.cpp
    fast.cpp
    icc.cpp
    uci.cpp
    legality.cpp
    queenscore.cpp
    tables.cpp
    w0_pawneval.cpp
    w0_search_w.cpp
    bishopscore.cpp
    egtb_lookup.cpp
    fics.cpp
    init.cpp
    tm_icc.cpp
    w0_quiescence_b.cpp
    w17.cpp
    book.cpp
    engine_impl.cpp
    hashcodes.cpp
    kingscore.cpp
    minibook.cpp
    rookscore.cpp
    w0.cpp
    w0_quiescence_w.cpp
    w17_quiescence.cpp
) 

add_executable(maxima main.cpp)

message(STATUS "Libraries: ${Boost_LIBRARIES} ${PostgreSQL_LIBRARIES} pthread") 

target_link_libraries(maxchess ${Boost_LIBRARIES} ${PostgreSQL_LIBRARIES} pthread)
target_link_libraries(maxima maxchess)
