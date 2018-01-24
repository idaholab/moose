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

#include "FunctionDirichletBC.h"
#include "Function.h"

template <>
InputParameters
validParams<FunctionDirichletBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addClassDescription(
      "Imposes the essential boundary condition $u=g(t,\\vec{x})$, where $g$ "
      "is a (possibly) time and space-dependent MOOSE Function.");
  return params;
}

FunctionDirichletBC::FunctionDirichletBC(const InputParameters & parameters)
  : NodalBC(parameters), _func(getFunction("function"))
{
}

Real
FunctionDirichletBC::f()
{
  return _func.value(_t, *_current_node);
}

Real
FunctionDirichletBC::computeQpResidual()
{
  return _u[_qp] - f();
}
