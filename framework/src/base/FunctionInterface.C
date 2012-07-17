/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "FunctionInterface.h"
#include "Function.h"
#include "Problem.h"
#include "MooseTypes.h"

FunctionInterface::FunctionInterface(InputParameters & params) :
    _fni_problem(*params.get<Problem *>("_problem")),
    _fni_tid(params.have_parameter<THREAD_ID>("_tid") ? params.get<THREAD_ID>("_tid") : 0),
    _fni_params(params)
{
}

Function &
FunctionInterface::getFunction(const std::string & name)
{
  return _fni_problem.getFunction(_fni_params.get<FunctionName>(name), _fni_tid);
}

Function &
FunctionInterface::getFunctionByName(const std::string & name)
{
  return _fni_problem.getFunction(name, _fni_tid);
}
