#include <Functions/registerFunctions.h>

#include <Functions/FunctionFactory.h>


#include <iostream>

namespace DB
{

/** These functions are defined in a separate translation units.
  * This is done in order to reduce the consumption of RAM during build, and to speed up the parallel build.
  */
void registerFunctionsArithmetic(FunctionFactory &);
void registerFunctionsArray(FunctionFactory &);
void registerFunctionsConversion(FunctionFactory &);
void registerFunctionsDateTime(FunctionFactory &);
void registerFunctionsMiscellaneous(FunctionFactory &);


void registerFunctions()
{
    auto & factory = FunctionFactory::instance();

    registerFunctionsArithmetic(factory);
    registerFunctionsArray(factory);
    registerFunctionsConversion(factory);
    registerFunctionsDateTime(factory);
    registerFunctionsMiscellaneous(factory);
}

}
