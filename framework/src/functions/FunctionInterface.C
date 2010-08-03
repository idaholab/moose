#include "Function.h"
#include "FunctionInterface.h"
#include "FunctionWarehouse.h"

FunctionInterface::FunctionInterface(FunctionWarehouse & func_warehouse, InputParameters & params):
  _func_warehouse(func_warehouse),
  _func_params(params)
{
}

Function &
FunctionInterface::getFunction( std::string name )
{
  return _func_warehouse.getFunction(_func_params.get<std::string>(name));
}
