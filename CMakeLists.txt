cmake_minimum_required (VERSION 2.8.4) 

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

set(PostgreSQL_ADDITIONAL_SEARCH_PATHS "/usr/include/postgresql/9.3/server")
find_package(PostgreSQL) 

include_directories("/usr/include/postgresql") 
include_directories("${PROJECT_BINARY_DIR}") 
include_directories(${PROJECT_SOURCE_DIR}/src)

add_subdirectory (src)  
add_subdirectory (test) 

enable_testing ()
add_test (NAME fenton COMMAND test/fenton)
