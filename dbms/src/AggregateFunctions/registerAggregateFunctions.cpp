#include <AggregateFunctions/registerAggregateFunctions.h>

#include <AggregateFunctions/AggregateFunctionFactory.h>

namespace DB
{

void registerAggregateFunctionAvg(AggregateFunctionFactory & factory);
void registerAggregateFunctionCount(AggregateFunctionFactory & factory);
void registerAggregateFunctionSum(AggregateFunctionFactory & factory);
void registerAggregateFunctionsUniq(AggregateFunctionFactory & factory);
void registerAggregateFunctionTopK(AggregateFunctionFactory & factory);


void registerAggregateFunctions()
{
    auto & factory = AggregateFunctionFactory::instance();

    registerAggregateFunctionAvg(factory);
    registerAggregateFunctionCount(factory);
    registerAggregateFunctionSum(factory);
    registerAggregateFunctionsUniq(factory);
    registerAggregateFunctionTopK(factory);
}

}
