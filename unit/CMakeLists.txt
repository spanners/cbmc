if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
   "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR
   "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"
)
    #  private is overwritten in the gdb_api test cases
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-keyword-macro")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    #   This would be the place to enable warnings for Windows builds, although
    #   config.inc doesn't seem to do that currently
endif()

file(GLOB_RECURSE sources "*.cpp" "*.h")

file(GLOB_RECURSE testing_utils "testing-utils/*.cpp" "testing-utils/*.h")

if(NOT WITH_MEMORY_ANALYZER)
    file(GLOB_RECURSE memory_analyzer_sources "memory-analyzer/*.cpp")
    list(REMOVE_ITEM sources ${memory_analyzer_sources})
endif()

list(REMOVE_ITEM sources
    # Don't build
    ${CMAKE_CURRENT_SOURCE_DIR}/elf_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/smt2_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/osx_fat_reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp_scanner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ieee_float.cpp

    # Will be built into a separate library and linked
    ${testing_utils}

    # Intended to fail to compile
    ${CMAKE_CURRENT_SOURCE_DIR}/util/expr_cast/expr_undefined_casts.cpp
)

add_subdirectory(testing-utils)

add_executable(unit ${sources})
target_include_directories(unit
    PUBLIC
    ${CBMC_BINARY_DIR}
    ${CBMC_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CUDD_INCLUDE}
)
target_link_libraries(
        unit
        testing-utils
        ansi-c
        solvers
        goto-checker
        goto-programs
        goto-instrument-lib
        goto-symex
        cbmc-lib
        json-symtab-language
        statement-list
)

add_test(
    NAME unit
    COMMAND $<TARGET_FILE:unit>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
set_tests_properties(unit PROPERTIES LABELS "CORE;CBMC")
