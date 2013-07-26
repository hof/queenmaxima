cmake_minimum_required (VERSION 2.6) 

project (maxima)

find_package(Git)
if (GIT_FOUND)
    execute_process(COMMAND git --git-dir ${PROJECT_SOURCE_DIR}/.git rev-parse  HEAD
        OUTPUT_VARIABLE REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Git revision is ${REVISION}")
else()
    set(REVISION "none")
endif()

# Warning when build is not an out-of-source build.
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" InSourceBuild)
if(InSourceBuild)
    message(WARNING  "Avoid building inside the source tree!")
    message(WARNING  "Create a separate build directory instead (i.e. 'build-qm') and call CMake from there: ")
    message(WARNING  "  mkdir ../build-qm && cd ../build-qm && cmake ${CMAKE_SOURCE_DIR}")
endif(InSourceBuild)

configure_file ( 
	"${PROJECT_SOURCE_DIR}/src/config.h.in" 
	"${PROJECT_BINARY_DIR}/config.h" 
)

include_directories("${PROJECT_BINARY_DIR}") 

include_directories(${PROJECT_SOURCE_DIR}/src)

find_package(Boost 1.49 COMPONENTS system REQUIRED) 
find_package(PostgreSQL 8.0 REQUIRED) 

include_directories(${PostgreSQL_INCLUDE_DIRS}) 
include_directories(${Boost_INCLUDE_DIRS})

add_executable(maxima src/main.cpp 
	src/attack.cpp 
	src/checkcheck.cpp
        src/extensions.cpp
        src/hash.cpp
        src/knightscore.cpp
        src/parser.cpp
        src/run_test.cpp
        src/w0_evaluate.cpp
        src/w0_search_b.cpp
        src/w17_search.cpp
        src/bbtables.cpp
        src/db_pg.cpp
        src/fast.cpp
        src/icc.cpp
        src/legality.cpp
        src/queenscore.cpp
        src/tables.cpp
        src/w0_pawneval.cpp
        src/w0_search_w.cpp
        src/bishopscore.cpp
        src/egtb_lookup.cpp
        src/fics.cpp
        src/init.cpp
        src/main.cpp
        src/tm_icc.cpp
        src/w0_quiescence_b.cpp
        src/w17.cpp
        src/book.cpp
        src/engine_impl.cpp
        src/hashcodes.cpp
        src/kingscore.cpp
        src/minibook.cpp
        src/rookscore.cpp
        src/w0.cpp
        src/w0_quiescence_w.cpp
        src/w17_quiescence.cpp
)

message(STATUS "Libraries: ${Boost_LIBRARIES} ${PostgreSQL_LIBRARIES} pthread") 

target_link_libraries(maxima ${Boost_LIBRARIES} ${PostgreSQL_LIBRARIES} pthread)

