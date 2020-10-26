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
  params.addClassDescription("Defines a Dirichlet boundary condition for finite volume method.");
  params.addRequiredParam<Real>("value", "value to enforce at the boundary face");
  params.registerSystemAttributeName("FVDirichletBC");
  return params;
}

FVDirichletBC::FVDirichletBC(const InputParameters & parameters)
  : FVBoundaryCondition(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _val(getParam<Real>("value"))
{
}

Real
FVDirichletBC::boundaryValue(const FaceInfo & /*fi*/) const
{
  return _val;
}
