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
