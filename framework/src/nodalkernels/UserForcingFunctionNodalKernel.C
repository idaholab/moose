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

#include "UserForcingFunctionNodalKernel.h"

#include "Function.h"

template<>
InputParameters validParams<UserForcingFunctionNodalKernel>()
{
  InputParameters params = validParams<NodalKernel>();
  params.addRequiredParam<FunctionName>("function", "The forcing function");
  return params;
}

UserForcingFunctionNodalKernel::UserForcingFunctionNodalKernel(const InputParameters & parameters) :
    NodalKernel(parameters),
    _func(getFunction("function"))
{
}

Real
UserForcingFunctionNodalKernel::computeQpResidual()
{
  return -_func.value(_t, (*_current_node));
}
