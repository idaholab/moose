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

#include "FunctionNeumannBC.h"
#include "Function.h"

template <>
InputParameters
validParams<FunctionNeumannBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<FunctionName>("function", "The function.");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=h(t,\\vec{x})$, "
                             "where $h$ is a (possibly) time and space-dependent MOOSE Function.");
  return params;
}

FunctionNeumannBC::FunctionNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _func(getFunction("function"))
{
}

Real
FunctionNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * _func.value(_t, _q_point[_qp]);
}
