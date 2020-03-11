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
  InputParameters params = FVBoundaryCondition::validParams();
  params.addRequiredParam<Real>("value", "value to enforce at the boundary face");
  return params;
}

FVDirichletBC::FVDirichletBC(const InputParameters & parameters) : FVBoundaryCondition(parameters)
{
}

Real
FVDirichletBC::boundaryValue(const FaceInfo & /*fi*/)
{
  return getParam<Real>("value");
}
