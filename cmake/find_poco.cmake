option (USE_INTERNAL_POCO_LIBRARY "Set to FALSE to use system poco library instead of bundled" ${NOT_UNBUNDLED})

if (NOT USE_INTERNAL_POCO_LIBRARY)
    find_package (Poco COMPONENTS Net NetSSL XML Data Crypto DataODBC MongoDB)
endif ()

if (Poco_INCLUDE_DIRS AND Poco_Foundation_LIBRARY)
    include_directories (${Poco_INCLUDE_DIRS})
else ()

    set (USE_INTERNAL_POCO_LIBRARY 1)

    include (${ClickHouseLite_SOURCE_DIR}/cmake/find_ltdl.cmake)

    list (APPEND Poco_INCLUDE_DIRS
        "${ClickHouseLite_SOURCE_DIR}/contrib/libpoco/Foundation/include/"
        "${ClickHouseLite_SOURCE_DIR}/contrib/libpoco/Util/include/"
        "${ClickHouseLite_SOURCE_DIR}/contrib/libpoco/Net/include/"
        "${ClickHouseLite_SOURCE_DIR}/contrib/libpoco/Data/include/"
        "${ClickHouseLite_SOURCE_DIR}/contrib/libpoco/XML/include/"
    )

    if (OPENSSL_FOUND)
        set (Poco_NetSSL_FOUND 1)
        set (Poco_NetSSL_LIBRARY PocoNetSSL)
        set (Poco_Crypto_LIBRARY PocoCrypto)
        list (APPEND Poco_INCLUDE_DIRS
            "${ClickHouseLite_SOURCE_DIR}/contrib/libpoco/NetSSL_OpenSSL/include/"
            "${ClickHouseLite_SOURCE_DIR}/contrib/libpoco/Crypto/include/"
        )
    endif ()

    if (USE_STATIC_LIBRARIES AND USE_INTERNAL_ZLIB_LIBRARY)
        list (APPEND Poco_INCLUDE_DIRS
            "${ClickHouseLite_SOURCE_DIR}/contrib/libzlib-ng/"
            "${ClickHouseLite_BINARY_DIR}/contrib/libzlib-ng/"
        )
    endif ()

    set (Poco_Foundation_LIBRARY PocoFoundation)
    set (Poco_Util_LIBRARY PocoUtil)
    set (Poco_Net_LIBRARY PocoNet)
    set (Poco_Data_LIBRARY PocoData)
    set (Poco_XML_LIBRARY PocoXML)
    include_directories (BEFORE ${Poco_INCLUDE_DIRS})
endif ()

message(STATUS "Using Poco: ${Poco_INCLUDE_DIRS} : ${Poco_Foundation_LIBRARY},${Poco_Util_LIBRARY},${Poco_Net_LIBRARY},${Poco_NetSSL_LIBRARY},${Poco_XML_LIBRARY},${Poco_Data_LIBRARY},${Poco_DataODBC_LIBRARY},${Poco_MongoDB_LIBRARY}; MongoDB=${Poco_MongoDB_FOUND}, DataODBC=${Poco_DataODBC_FOUND}, NetSSL=${Poco_NetSSL_FOUND}")
