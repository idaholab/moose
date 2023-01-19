//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVDirichletBC.h"

registerMooseObject("MooseApp", FVDirichletBC);

InputParameters
FVDirichletBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription("Defines a Dirichlet boundary condition for finite volume method.");
  params.addRequiredParam<Real>("value", "value to enforce at the boundary face");
  return params;
}

FVDirichletBC::FVDirichletBC(const InputParameters & parameters)
  : FVDirichletBCBase(parameters), _val(getParam<Real>("value"))
{
}

ADReal
FVDirichletBC::boundaryValue(const FaceInfo & /*fi*/) const
{
  return _val;
}
