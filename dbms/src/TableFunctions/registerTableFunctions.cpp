#include <TableFunctions/registerTableFunctions.h>
#include <TableFunctions/TableFunctionFactory.h>


namespace DB
{

void registerTableFunctionNumbers(TableFunctionFactory & factory);


void registerTableFunctions()
{
    auto & factory = TableFunctionFactory::instance();

    registerTableFunctionNumbers(factory);
}

}
