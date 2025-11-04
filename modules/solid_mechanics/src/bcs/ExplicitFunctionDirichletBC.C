//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExplicitFunctionDirichletBC.h"
#include "Function.h"

registerMooseObject("SolidMechanicsApp", ExplicitFunctionDirichletBC);
registerMooseObjectRenamed("SolidMechanicsApp",
                           DirectFunctionDirichletBC,
                           "10/14/2025 00:00",
                           ExplicitFunctionDirichletBC);

InputParameters
ExplicitFunctionDirichletBC::validParams()
{
  InputParameters params = ExplicitDirichletBCBase::validParams();
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addClassDescription(
      "Imposes the essential boundary condition $u=g(t,\\vec{x})$, where $g$ "
      "is a (possibly) time and space-dependent MOOSE Function.");
  return params;
}

ExplicitFunctionDirichletBC::ExplicitFunctionDirichletBC(const InputParameters & parameters)
  : ExplicitDirichletBCBase(parameters), _func(getFunction("function"))
{
}

Real
ExplicitFunctionDirichletBC::computeQpValue()
{
  return _func.value(_t, *_current_node);
}
