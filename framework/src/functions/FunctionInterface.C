#include "Function.h"
#include "FunctionInterface.h"
#include "FunctionWarehouse.h"

FunctionInterface::FunctionInterface(FunctionWarehouse & func_warehouse, InputParameters params, THREAD_ID tid):
  _func_warehouse(func_warehouse),
  _func_params(params),
  _func_tid(tid)
{
}

Function &
FunctionInterface::getFunction( std::string name )
{
  return _func_warehouse.getFunction(_func_tid, _func_params.get<std::string>(name));
}
