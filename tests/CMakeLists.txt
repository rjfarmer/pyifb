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

function(append_existing_dirs out_var)
    foreach(_dir IN LISTS ARGN)
        if(_dir AND IS_DIRECTORY "${_dir}")
            list(APPEND ${out_var} "${_dir}")
        endif()
    endforeach()
    set(${out_var} "${${out_var}}" PARENT_SCOPE)
endfunction()

function(append_resource_runtime_dirs out_var compiler_path)
    if(NOT compiler_path)
        set(${out_var} "${${out_var}}" PARENT_SCOPE)
        return()
    endif()

    execute_process(
        COMMAND "${compiler_path}" -print-resource-dir
        OUTPUT_VARIABLE _resource_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _resource_dir_status
    )

    if(_resource_dir_status EQUAL 0 AND _resource_dir)
        execute_process(
            COMMAND uname -m
            OUTPUT_VARIABLE _arch
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        append_existing_dirs(
            ${out_var}
            "${_resource_dir}/lib/${_arch}-redhat-linux-gnu"
            "${_resource_dir}/lib/${CMAKE_SYSTEM_PROCESSOR}-unknown-linux-gnu"
            "${_resource_dir}/lib/${CMAKE_LIBRARY_ARCHITECTURE}"
            "${_resource_dir}/lib/linux"
            "${_resource_dir}/lib"
        )
    else()
        set(${out_var} "${${out_var}}" PARENT_SCOPE)
    endif()
endfunction()

function(find_flang_runtime out_link_dir out_link_name)
    set(_search_dirs "")

    if(DEFINED ENV{FLANG_RT_DIR} AND EXISTS "$ENV{FLANG_RT_DIR}")
        append_existing_dirs(_search_dirs "$ENV{FLANG_RT_DIR}")
    endif()

    find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config)
    if(LLVM_CONFIG_EXECUTABLE)
        execute_process(
            COMMAND "${LLVM_CONFIG_EXECUTABLE}" --libdir
            OUTPUT_VARIABLE _llvm_libdir
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        execute_process(
            COMMAND "${LLVM_CONFIG_EXECUTABLE}" --prefix
            OUTPUT_VARIABLE _llvm_prefix
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        append_existing_dirs(
            _search_dirs
            "${_llvm_libdir}"
            "${_llvm_prefix}/lib"
            "${_llvm_prefix}/lib64"
        )
    endif()

    append_resource_runtime_dirs(_search_dirs "${CMAKE_Fortran_COMPILER}")
    append_resource_runtime_dirs(_search_dirs "${CMAKE_C_COMPILER}")

    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE _arch
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    file(GLOB _clang_dirs LIST_DIRECTORIES true "/usr/lib/clang/*/lib/${_arch}-redhat-linux-gnu")
    file(GLOB _llvm_dirs LIST_DIRECTORIES true "/usr/lib/llvm-*/lib")
    append_existing_dirs(_search_dirs ${_clang_dirs} ${_llvm_dirs} "/usr/lib64" "/usr/lib")

    list(REMOVE_DUPLICATES _search_dirs)

    set(_chosen_dir "")
    set(_chosen_lib "")
    foreach(_dir IN LISTS _search_dirs)
        if(EXISTS "${_dir}/libflang_rt.runtime.so" OR EXISTS "${_dir}/libflang_rt.runtime.a")
            set(_chosen_dir "${_dir}")
            set(_chosen_lib flang_rt.runtime)
        elseif(EXISTS "${_dir}/libFortranRuntime.so" OR EXISTS "${_dir}/libFortranRuntime.a")
            set(_chosen_dir "${_dir}")
            set(_chosen_lib FortranRuntime)
        endif()

        if(_chosen_lib)
            break()
        endif()
    endforeach()

    if(NOT _chosen_lib)
        set(_chosen_lib FortranRuntime)
    endif()

    set(${out_link_dir} "${_chosen_dir}" PARENT_SCOPE)
    set(${out_link_name} "${_chosen_lib}" PARENT_SCOPE)
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
    find_flang_runtime(_flang_runtime_dir _flang_runtime_name)
    if(_flang_runtime_dir)
        target_link_directories(cdesc_test_helper PRIVATE "${_flang_runtime_dir}")
    endif()
    target_link_libraries(cdesc_test_helper PRIVATE "${_flang_runtime_name}")
else()
    target_link_libraries(cdesc_test_helper PRIVATE gfortran)
endif()

add_custom_target(extract_mods ALL DEPENDS ${_extract_outputs})