//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFunctionDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", FVFunctionDirichletBC);

InputParameters
FVFunctionDirichletBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addRequiredParam<FunctionName>("function", "The exact solution function.");
  params.addClassDescription(
      "Imposes the essential boundary condition $u=g(t,\\vec{x})$, where $g$ "
      "is a (possibly) time and space-dependent MOOSE Function.");
  return params;
}

FVFunctionDirichletBC::FVFunctionDirichletBC(const InputParameters & parameters)
  : FVDirichletBCBase(parameters), _function(getFunction("function"))
{
}

ADReal
FVFunctionDirichletBC::boundaryValue(const FaceInfo & fi, const Moose::StateArg & state) const
{

  if (state.state != 0 && state.iteration_type == Moose::SolutionIterationType::Time)
  {
    mooseAssert(state.state == 1, "We cannot access values beyond the previous time step.");
    return _function.value(_t_old, fi.faceCentroid());
  }
  else
    return _function.value(_t, fi.faceCentroid());
}
