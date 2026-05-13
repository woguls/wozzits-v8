cmake_minimum_required(VERSION 3.20)

project(wozzits_v8 LANGUAGES CXX)

option(WOZZITS_V8_BUILD_TESTS "Build wozzits-v8 tests" ON)
option(WOZZITS_V8_BUILD_EXAMPLES "Build wozzits-v8 examples" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(wozzits_apply_v8_libcxx target)
    target_include_directories(${target}
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/generated/libcxx"
            "${V8_ROOT}/third_party/libc++/src/include"
            "${V8_ROOT}/third_party/libc++abi/src/include"
    )

    target_compile_definitions(${target}
        PRIVATE
            _LIBCPP_ABI_NAMESPACE=__Cr
            _LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS
            _LIBCXXABI_DISABLE_VISIBILITY_ANNOTATIONS

            V8_COMPRESS_POINTERS
            V8_COMPRESS_POINTERS_IN_SHARED_CAGE
            V8_31BIT_SMIS_ON_64BIT_ARCH
    )
endfunction()

if(MSVC)
   set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(V8 REQUIRED)

add_library(wozzits_v8 STATIC
    include/wozzits/script/script_result.h
    include/wozzits/script/script_host.h
    include/wozzits/script/v8_script_host.h

    src/v8_script_host.cpp
)

add_library(wozzits_v8::wozzits_v8 ALIAS wozzits_v8)

target_include_directories(wozzits_v8
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/generated/libcxx"
        "${V8_ROOT}/third_party/libc++/src/include"
        "${V8_ROOT}/third_party/libc++abi/src/include"
)





target_link_libraries(wozzits_v8
    PRIVATE
        V8::v8
)

target_compile_features(wozzits_v8 PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(wozzits_v8
        PUBLIC
            /std:c++20
            /Zc:__cplusplus
    )

    target_compile_definitions(wozzits_v8
        PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX

            V8_COMPRESS_POINTERS
            V8_COMPRESS_POINTERS_IN_SHARED_CAGE
            V8_31BIT_SMIS_ON_64BIT_ARCH

            _LIBCPP_ABI_NAMESPACE=__Cr
    )
endif()

wozzits_apply_v8_libcxx(wozzits_v8)

if(WOZZITS_V8_BUILD_TESTS)
    enable_testing()

    add_executable(script_host_smoke
        tests/script_host/script_host_smoke.cpp
    )

    target_link_libraries(script_host_smoke
        PRIVATE
            wozzits_v8::wozzits_v8
    )

    target_compile_features(script_host_smoke PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(script_host_smoke
            PRIVATE
                /std:c++20
                /Zc:__cplusplus
        )
    endif()

    wozzits_apply_v8_libcxx(script_host_smoke)

    add_test(NAME script_host_smoke COMMAND script_host_smoke)
endif()