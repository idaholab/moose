#include "FunctionInterface.h"
#include "Function.h"
#include "Problem.h"

FunctionInterface::FunctionInterface(InputParameters & params) :
    _problem(*params.get<Problem *>("_problem")),
    _tid(params.have_parameter<THREAD_ID>("_tid") ? params.get<THREAD_ID>("_tid") : 0),
    _params(params)
{
}

Function &
FunctionInterface::getFunction(const std::string & name)
{
  return _problem.getFunction(_params.get<std::string>(name), _tid);
}
