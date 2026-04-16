cmake_minimum_required(VERSION 3.18)

project(pyifb_test_helpers LANGUAGES C Fortran)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

get_filename_component(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
get_filename_component(REPO_ROOT "${TESTS_DIR}/.." ABSOLUTE)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${TESTS_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${TESTS_DIR}")
set(CMAKE_Fortran_MODULE_DIRECTORY "${TESTS_DIR}")

file(GLOB FORTRAN_SOURCES CONFIGURE_DEPENDS "${TESTS_DIR}/*.f90")

if(NOT FORTRAN_SOURCES)
    message(FATAL_ERROR "No Fortran test sources found in ${TESTS_DIR}")
endif()

set(IS_FLANG FALSE)
if(CMAKE_Fortran_COMPILER_ID MATCHES "^(LLVMFlang|Flang)$")
    set(IS_FLANG TRUE)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "IntelLLVM")
    set(FORTRAN_OPTIONS -O2 --warn-all -fcheck=all)
elseif(IS_FLANG)
    set(FORTRAN_OPTIONS -O2)
else()
    set(
        FORTRAN_OPTIONS
        -ggdb
        -D_FORTIFY_SOURCE=2
        -ffree-line-length-none
        -ffree-form
        -fstack-clash-protection
        -fstack-protector-all
        -fstack-protector
    )
endif()

function(get_command_output out_var)
    execute_process(
        COMMAND ${ARGN}
        OUTPUT_VARIABLE _output
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _status
    )

    if(_status EQUAL 0 AND _output)
        set(${out_var} "${_output}" PARENT_SCOPE)
    else()
        set(${out_var} "" PARENT_SCOPE)
    endif()
endfunction()

function(find_clang_include_dir out_var compiler_path)
    if(NOT compiler_path)
        set(${out_var} "" PARENT_SCOPE)
        return()
    endif()

    get_command_output(_include_dir "${compiler_path}" -print-file-name=include)
    if(_include_dir AND IS_DIRECTORY "${_include_dir}" AND EXISTS "${_include_dir}/ISO_Fortran_binding.h")
        set(${out_var} "${_include_dir}" PARENT_SCOPE)
    else()
        set(${out_var} "" PARENT_SCOPE)
    endif()
endfunction()

function(find_gfortran_include_dir out_var)
    find_program(_gfortran_exe gfortran)
    if(NOT _gfortran_exe)
        set(${out_var} "" PARENT_SCOPE)
        return()
    endif()

    get_command_output(_include_dir "${_gfortran_exe}" -print-file-name=include)
    if(_include_dir AND IS_DIRECTORY "${_include_dir}" AND EXISTS "${_include_dir}/ISO_Fortran_binding.h")
        set(${out_var} "${_include_dir}" PARENT_SCOPE)
    else()
        set(${out_var} "" PARENT_SCOPE)
    endif()
endfunction()

set(_extract_outputs "")
foreach(fortran_source IN LISTS FORTRAN_SOURCES)
    get_filename_component(target_name "${fortran_source}" NAME_WE)

    add_library(${target_name} SHARED "${fortran_source}")
    set_target_properties(
        ${target_name}
        PROPERTIES
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY "${TESTS_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${TESTS_DIR}"
            Fortran_MODULE_DIRECTORY "${TESTS_DIR}"
    )
    target_compile_options(${target_name} PRIVATE -cpp ${FORTRAN_OPTIONS})

    set(module_file "${TESTS_DIR}/${target_name}.mod")
    set(extract_file "${TESTS_DIR}/${target_name}.gz.extract")
    list(APPEND _extract_outputs "${extract_file}")

    add_custom_command(
        OUTPUT "${extract_file}"
        COMMAND ${CMAKE_COMMAND} -E copy "${module_file}" "${module_file}.gz"
        COMMAND gunzip -f -c "${module_file}.gz" > "${extract_file}"
        COMMAND ${CMAKE_COMMAND} -E rm -f "${module_file}.gz"
        DEPENDS ${target_name}
        VERBATIM
    )
endforeach()

add_library(cdesc_test_helper SHARED "${TESTS_DIR}/cdesc_test_helper.c")
set_target_properties(
    cdesc_test_helper
    PROPERTIES
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY "${TESTS_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${TESTS_DIR}"
)
target_include_directories(cdesc_test_helper PRIVATE "${REPO_ROOT}")

if(CMAKE_C_COMPILER_ID STREQUAL "IntelLLVM")
    target_link_options(cdesc_test_helper PRIVATE -fortlib)
elseif(IS_FLANG)
    set(_clang_compiler "${CMAKE_C_COMPILER}")
    if(NOT _clang_compiler)
        set(_clang_compiler "${CMAKE_Fortran_COMPILER}")
    endif()

    # Use GCC's ISO_Fortran_binding.h when linking libgfortran to keep ABI consistent.
    find_gfortran_include_dir(_gfortran_include_dir)
    if(_gfortran_include_dir)
        target_include_directories(cdesc_test_helper PRIVATE "${_gfortran_include_dir}")
    else()
        find_clang_include_dir(_clang_include_dir "${_clang_compiler}")
        if(_clang_include_dir)
            target_include_directories(cdesc_test_helper PRIVATE "${_clang_include_dir}")
        endif()
    endif()

    target_link_libraries(cdesc_test_helper PRIVATE gfortran)
else()
    target_link_libraries(cdesc_test_helper PRIVATE gfortran)
endif()

add_custom_target(extract_mods ALL DEPENDS ${_extract_outputs})