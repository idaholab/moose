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

// MOOSE includes
#include "ElementL2ErrorFunctionAux.h"
#include "Function.h"

template <>
InputParameters
validParams<ElementL2ErrorFunctionAux>()
{
  InputParameters params = validParams<ElementLpNormAux>();
  params.addRequiredParam<FunctionName>("function", "Function representing the exact solution");
  return params;
}

ElementL2ErrorFunctionAux::ElementL2ErrorFunctionAux(const InputParameters & parameters)
  : ElementLpNormAux(parameters), _func(getFunction("function"))
{
}

Real
ElementL2ErrorFunctionAux::computeValue()
{
  return _func.value(_t, _q_point[_qp]) - _coupled_var[_qp];
}
