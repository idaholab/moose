#include "FunctionInterface.h"
#include "Function.h"
#include "SubProblem.h"

namespace Moose {
} // namespace

FunctionInterface::FunctionInterface(InputParameters & params) :
    _func_problem(*params.get<Moose::SubProblem *>("_subproblem")),
    _func_tid(params.have_parameter<THREAD_ID>("_tid") ? params.get<THREAD_ID>("_tid") : 0),
    _func_params(params)
{
}

Function &
FunctionInterface::getFunction( const std::string & name )
{
  return _func_problem.getFunction(_func_params.get<std::string>(name), _func_tid);
}

