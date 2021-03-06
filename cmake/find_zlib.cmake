option (USE_INTERNAL_ZLIB_LIBRARY "Set to FALSE to use system zlib library instead of bundled" ${NOT_UNBUNDLED})

if (NOT USE_INTERNAL_ZLIB_LIBRARY)
    find_package (ZLIB)
endif ()

if (NOT ZLIB_FOUND)
    set (USE_INTERNAL_ZLIB_LIBRARY 1)
    set (ZLIB_INCLUDE_DIR "${ClickHouseLite_SOURCE_DIR}/contrib/libzlib-ng")
    if (USE_STATIC_LIBRARIES)
        set (ZLIB_LIBRARIES zlibstatic)
    else ()
        set (ZLIB_LIBRARIES zlib)
    endif ()
endif ()

message (STATUS "Using zlib: ${ZLIB_INCLUDE_DIR} : ${ZLIB_LIBRARIES}")
