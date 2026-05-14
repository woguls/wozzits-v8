# FindV8.cmake
#
# Finds a locally built V8 monolith build.
#
# Expected layout:
#
#   V8_ROOT/
#     include/v8.h
#     out/x64.release.monolith/
#       obj/v8_monolith.lib
#       obj/v8_libbase.lib
#       obj/v8_libplatform.lib
#       obj/third_party/partition_alloc/src/partition_alloc/allocator_base.lib
#       obj/third_party/partition_alloc/src/partition_alloc/allocator_core.lib
#       obj/third_party/partition_alloc/src/partition_alloc/allocator_shim.lib
#       obj/third_party/zlib/zlib.lib
#       obj/third_party/zlib/google/compression_utils_portable.lib
#
# Defines:
#
#   V8::v8

include(FindPackageHandleStandardArgs)

set(V8_ROOT "" CACHE PATH "Path to V8 checkout root")
set(V8_INCLUDE_DIR "" CACHE PATH "Path to V8 include directory")
set(V8_LIB_DIR "" CACHE PATH "Path to V8 obj library directory")

if(NOT V8_INCLUDE_DIR AND V8_ROOT)
    set(V8_INCLUDE_DIR "${V8_ROOT}/include" CACHE PATH "Path to V8 include directory" FORCE)
endif()

if(NOT V8_LIB_DIR AND V8_ROOT)
    set(V8_LIB_DIR "${V8_ROOT}/out/x64.release.monolith/obj" CACHE PATH "Path to V8 obj library directory" FORCE)
endif()

find_path(V8_INCLUDE_DIR
    NAMES v8.h
    HINTS "${V8_INCLUDE_DIR}" "${V8_ROOT}/include"
    NO_DEFAULT_PATH
)

find_library(V8_MONOLITH_LIBRARY
    NAMES v8_monolith
    HINTS "${V8_LIB_DIR}"
    NO_DEFAULT_PATH
)

find_library(V8_LIBBASE_LIBRARY
    NAMES v8_libbase
    HINTS "${V8_LIB_DIR}"
    NO_DEFAULT_PATH
)

find_library(V8_LIBPLATFORM_LIBRARY
    NAMES v8_libplatform
    HINTS "${V8_LIB_DIR}"
    NO_DEFAULT_PATH
)

find_library(V8_ALLOCATOR_BASE_LIBRARY
    NAMES allocator_base
    HINTS "${V8_LIB_DIR}/third_party/partition_alloc/src/partition_alloc"
    NO_DEFAULT_PATH
)

find_library(V8_ALLOCATOR_CORE_LIBRARY
    NAMES allocator_core
    HINTS "${V8_LIB_DIR}/third_party/partition_alloc/src/partition_alloc"
    NO_DEFAULT_PATH
)

find_library(V8_ALLOCATOR_SHIM_LIBRARY
    NAMES allocator_shim
    HINTS "${V8_LIB_DIR}/third_party/partition_alloc/src/partition_alloc"
    NO_DEFAULT_PATH
)

find_library(V8_ZLIB_LIBRARY
    NAMES zlib
    HINTS "${V8_LIB_DIR}/third_party/zlib"
    NO_DEFAULT_PATH
)

find_library(V8_COMPRESSION_UTILS_LIBRARY
    NAMES compression_utils_portable
    HINTS "${V8_LIB_DIR}/third_party/zlib/google"
    NO_DEFAULT_PATH
)

find_library(V8_LIBCXX_LIBRARY
    NAMES libc++
    HINTS
        "${V8_ROOT}/out/x64.release.monolith/obj/buildtools/third_party/libc++"
    NO_DEFAULT_PATH
)

find_package_handle_standard_args(V8
    REQUIRED_VARS
        V8_INCLUDE_DIR
        V8_MONOLITH_LIBRARY
        V8_LIBBASE_LIBRARY
        V8_LIBPLATFORM_LIBRARY
)

if(V8_FOUND AND NOT TARGET V8::v8)
    add_library(V8::v8 UNKNOWN IMPORTED)

    set_target_properties(V8::v8 PROPERTIES
        IMPORTED_LOCATION "${V8_MONOLITH_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${V8_INCLUDE_DIR}"
    )

    set_property(TARGET V8::v8 APPEND PROPERTY
    INTERFACE_LINK_DIRECTORIES
        "${V8_ROOT}/out/x64.release.monolith/obj/buildtools/third_party/libc++"
)

    set(_v8_extra_libs
        "${V8_LIBPLATFORM_LIBRARY}"
        "${V8_LIBBASE_LIBRARY}"
    )

    if(V8_LIBCXX_LIBRARY)
        list(APPEND _v8_extra_libs "${V8_LIBCXX_LIBRARY}")
    endif()

    if(V8_ALLOCATOR_BASE_LIBRARY)
        list(APPEND _v8_extra_libs "${V8_ALLOCATOR_BASE_LIBRARY}")
    endif()

    if(V8_ALLOCATOR_CORE_LIBRARY)
        list(APPEND _v8_extra_libs "${V8_ALLOCATOR_CORE_LIBRARY}")
    endif()

    if(V8_ALLOCATOR_SHIM_LIBRARY)
        list(APPEND _v8_extra_libs "${V8_ALLOCATOR_SHIM_LIBRARY}")
    endif()

    if(V8_ZLIB_LIBRARY)
        list(APPEND _v8_extra_libs "${V8_ZLIB_LIBRARY}")
    endif()

    if(V8_COMPRESSION_UTILS_LIBRARY)
        list(APPEND _v8_extra_libs "${V8_COMPRESSION_UTILS_LIBRARY}")
    endif()

if(WIN32)
    list(APPEND _v8_extra_libs
        winmm
        dbghelp
        shlwapi
        ws2_32
        advapi32
        userenv
        secur32
        shell32
        ole32
        oleaut32
        uuid

    )
endif()

    set_property(TARGET V8::v8 APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES
            ${_v8_extra_libs}
    )
endif()

mark_as_advanced(
    V8_ROOT
    V8_INCLUDE_DIR
    V8_LIB_DIR
    V8_MONOLITH_LIBRARY
    V8_LIBBASE_LIBRARY
    V8_LIBPLATFORM_LIBRARY
    V8_ALLOCATOR_BASE_LIBRARY
    V8_ALLOCATOR_CORE_LIBRARY
    V8_ALLOCATOR_SHIM_LIBRARY
    V8_ZLIB_LIBRARY
    V8_COMPRESSION_UTILS_LIBRARY
    V8_LIBCXX_LIBRARY
)