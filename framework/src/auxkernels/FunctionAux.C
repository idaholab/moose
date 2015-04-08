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

#include "FunctionAux.h"
#include "Function.h"

template<>
InputParameters validParams<FunctionAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<FunctionName>("function", "The function to use as the value");
  return params;
}

FunctionAux::FunctionAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _func(getFunction("function"))
{
}

Real
FunctionAux::computeValue()
{
  if (isNodal())
    return _func.value(_t, *_current_node);
  else
    return _func.value(_t, _q_point[_qp]);
}
