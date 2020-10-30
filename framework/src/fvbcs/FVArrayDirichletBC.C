//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVArrayDirichletBC.h"

registerMooseObject("MooseApp", FVArrayDirichletBC);

InputParameters
FVArrayDirichletBC::validParams()
{
  InputParameters params = FVBoundaryCondition::validParams();
  params.addRequiredParam<RealEigenVector>("value", "value to enforce at the boundary face");
  params.registerSystemAttributeName("FVDirichletBC");
  return params;
}

FVArrayDirichletBC::FVArrayDirichletBC(const InputParameters & parameters)
  : FVBoundaryCondition(parameters),
    MooseVariableInterface<RealEigenVector>(this,
                                            false,
                                            "variable",
                                            Moose::VarKindType::VAR_NONLINEAR,
                                            Moose::VarFieldType::VAR_FIELD_ARRAY),
    _val(getParam<RealEigenVector>("value"))
{
}

RealEigenVector
FVArrayDirichletBC::boundaryValue(const FaceInfo & /*fi*/) const
{
  return _val;
}
