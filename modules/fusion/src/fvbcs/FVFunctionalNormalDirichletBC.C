//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFunctionalNormalDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", FVFunctionalNormalDirichletBC);

InputParameters
FVFunctionalNormalDirichletBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addRequiredParam<FunctionName>("function", "The exact solution function.");
  MooseEnum direction("x y z");
  params.addRequiredParam<MooseEnum>(
      "direction", direction, "The direction a given velocity should be projected onto");
  params.addClassDescription(
      "Imposes the essential boundary condition $u=g(t,\\vec{x})$, where $g$ "
      "is a (possibly) time and space-dependent MOOSE Function.");
  return params;
}

FVFunctionalNormalDirichletBC::FVFunctionalNormalDirichletBC(const InputParameters & parameters)
  : FVDirichletBCBase(parameters),
    _direction(getParam<MooseEnum>("direction")),
    _function(getFunction("function"))
{
}

ADReal
FVFunctionalNormalDirichletBC::boundaryValue(const FaceInfo & fi) const
{
  return _function.value(_t, fi.faceCentroid()) * (-fi.normal()(_direction));
}
