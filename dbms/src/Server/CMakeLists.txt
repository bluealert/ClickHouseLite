# 'clickhouse' binary is a multi purpose tool,
# that contain multiple execution modes (client, server, etc.)
# each of them is built and linked as a separate library, defined below.

add_library(clickhouse-server
    ConfigReloader.cpp
    HTTPHandler.cpp
    MetricsTransmitter.cpp
    NotFoundHandler.cpp
    PingRequestHandler.cpp
    RootRequestHandler.cpp
    Server.cpp
    StatusFile.cpp
    TCPHandler.cpp
    )

target_link_libraries(clickhouse-server daemon clickhouse_storages_system clickhouse_functions clickhouse_aggregate_functions clickhouse_table_functions)
target_include_directories (clickhouse-server PUBLIC ${ClickHouse_SOURCE_DIR}/libs/libdaemon/include)

add_library (clickhouse-client Client.cpp)
target_link_libraries (clickhouse-client dbms clickhouse_functions clickhouse_aggregate_functions ${LINE_EDITING_LIBS} ${Boost_PROGRAM_OPTIONS_LIBRARY})
install (FILES clickhouse-client.xml DESTINATION ${CLICKHOUSE_ETC_DIR}/clickhouse-client COMPONENT clickhouse-client RENAME config.xml)

add_executable(clickhouse main.cpp)
target_include_directories(clickhouse PRIVATE ${COMMON_INCLUDE_DIR})
target_link_libraries(clickhouse
    clickhouse-server
    clickhouse-client)
INSTALL(TARGETS clickhouse RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT clickhouse)
# make symbolic links to concrete clickhouse applications
macro(install_symlink_to_clickhouse app)
    INSTALL(CODE "execute_process(COMMAND ln -sf clickhouse ${app} WORKING_DIRECTORY \"\$ENV{DESTDIR}${CMAKE_INSTALL_FULL_BINDIR}\" )" COMPONENT ${app})
endmacro(install_symlink_to_clickhouse)

install_symlink_to_clickhouse(clickhouse-server)
install_symlink_to_clickhouse(clickhouse-client)

INSTALL(
    FILES config.xml users.xml
    DESTINATION  ${CLICKHOUSE_ETC_DIR}/clickhouse-server
    COMPONENT clickhouse)
