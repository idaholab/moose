//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
