#include <Interpreters/Context.h>
#include <Interpreters/InterpreterCheckQuery.h>
#include <Parsers/ASTCheckQuery.h>
// #include <Storages/StorageDistributed.h>
#include <DataStreams/OneBlockInputStream.h>
#include <DataStreams/UnionBlockInputStream.h>
#include <DataTypes/DataTypeString.h>
#include <DataTypes/DataTypesNumber.h>
#include <Columns/ColumnString.h>
#include <Columns/ColumnsNumber.h>
#include <Common/typeid_cast.h>

#include <openssl/sha.h>
#include <deque>
#include <array>

namespace DB
{

namespace ErrorCodes
{
    extern const int INVALID_BLOCK_EXTRA_INFO;
    extern const int RECEIVED_EMPTY_DATA;
}


namespace
{

/// A helper structure for performing a response to a DESCRIBE TABLE query with a Distributed table.
/// Contains information about the local table that was retrieved from a single replica.
struct TableDescription
{
    TableDescription(const Block & block, const BlockExtraInfo & extra_info_)
        : extra_info(extra_info_)
    {
        const auto & name_column = typeid_cast<const ColumnString &>(*block.getByName("name").column);
        const auto & type_column = typeid_cast<const ColumnString &>(*block.getByName("type").column);
        const auto & default_type_column = typeid_cast<const ColumnString &>(*block.getByName("default_type").column);
        const auto & default_expression_column = typeid_cast<const ColumnString &>(*block.getByName("default_expression").column);

        size_t row_count = block.rows();

        names_with_types.reserve(name_column.byteSize() + type_column.byteSize() + (3 * row_count));

        SHA512_CTX ctx;
        SHA512_Init(&ctx);

        bool is_first = true;
        for (size_t i = 0; i < row_count; ++i)
        {
            const auto & name = name_column.getDataAt(i).toString();
            const auto & type = type_column.getDataAt(i).toString();
            const auto & default_type = default_type_column.getDataAt(i).toString();
            const auto & default_expression = default_expression_column.getDataAt(i).toString();

            names_with_types.append(is_first ? "" : ", ");
            names_with_types.append(name);
            names_with_types.append(" ");
            names_with_types.append(type);

            SHA512_Update(&ctx, reinterpret_cast<const unsigned char *>(name.data()), name.size());
            SHA512_Update(&ctx, reinterpret_cast<const unsigned char *>(type.data()), type.size());
            SHA512_Update(&ctx, reinterpret_cast<const unsigned char *>(default_type.data()), default_type.size());
            SHA512_Update(&ctx, reinterpret_cast<const unsigned char *>(default_expression.data()), default_expression.size());

            is_first = false;
        }

        SHA512_Final(hash.data(), &ctx);
    }

    using Hash = std::array<unsigned char, SHA512_DIGEST_LENGTH>;

    BlockExtraInfo extra_info;
    std::string names_with_types;
    Hash hash;
    UInt32 structure_class;
};

inline bool operator<(const TableDescription & lhs, const TableDescription & rhs)
{
    return lhs.hash < rhs.hash;
}

using TableDescriptions = std::deque<TableDescription>;

}

InterpreterCheckQuery::InterpreterCheckQuery(const ASTPtr & query_ptr_, const Context & context_)
    : query_ptr(query_ptr_), context(context_)
{
}

Block InterpreterCheckQuery::getSampleBlock() const
{
    Block block;
    ColumnWithTypeAndName col;

    col.name = "status";
    col.type = std::make_shared<DataTypeUInt8>();
    col.column = col.type->createColumn();
    block.insert(col);

    col.name = "host_name";
    col.type = std::make_shared<DataTypeString>();
    col.column = col.type->createColumn();
    block.insert(col);

    col.name = "host_address";
    col.type = std::make_shared<DataTypeString>();
    col.column = col.type->createColumn();
    block.insert(col);

    col.name = "port";
    col.type = std::make_shared<DataTypeUInt16>();
    col.column = col.type->createColumn();
    block.insert(col);

    col.name = "user";
    col.type = std::make_shared<DataTypeString>();
    col.column = col.type->createColumn();
    block.insert(col);

    col.name = "structure_class";
    col.type = std::make_shared<DataTypeUInt32>();
    col.column = col.type->createColumn();
    block.insert(col);

    col.name = "structure";
    col.type = std::make_shared<DataTypeString>();
    col.column = col.type->createColumn();
    block.insert(col);

    return block;
}


BlockIO InterpreterCheckQuery::execute()
{
    ASTCheckQuery & alter = typeid_cast<ASTCheckQuery &>(*query_ptr);
    String & table_name = alter.table;
    String database_name = alter.database.empty() ? context.getCurrentDatabase() : alter.database;

    StoragePtr table = context.getTable(database_name, table_name);

    result = Block{{ std::make_shared<ColumnUInt8>(), std::make_shared<DataTypeUInt8>(), "result" }};
    // result.safeGetByPosition(0).column->insert(Field(UInt64(table->checkData())));

    BlockIO res;
    res.in = std::make_shared<OneBlockInputStream>(result);
    res.in_sample = result.cloneEmpty();

    return res;
}

}
