//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionLibmeshDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionLibmeshDirichletBC);

InputParameters
FunctionLibmeshDirichletBC::validParams()
{
  InputParameters params = LibmeshDirichletBCBase::validParams();
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addClassDescription(
      "Imposes the essential boundary condition $u=g(t,\\vec{x})$, where $g$ is a (possibly) time "
      "and space-dependent MOOSE Function, through libMesh's constraint system, which projects $g$ "
      "onto the variable's boundary trace space.");
  return params;
}

FunctionLibmeshDirichletBC::FunctionLibmeshDirichletBC(const InputParameters & parameters)
  : LibmeshDirichletBCBase(parameters), _func(getFunction("function"))
{
}

Real
FunctionLibmeshDirichletBC::value(const libMesh::Point & p, const Real time) const
{
  return _func.value(time, p);
}
