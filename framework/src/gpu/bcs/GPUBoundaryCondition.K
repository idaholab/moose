//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUBoundaryCondition.h"

InputParameters
GPUBoundaryCondition::validParams()
{
  InputParameters params = GPUResidualObject::validParams();
  params += BoundaryRestrictableRequired::validParams();

  params.registerBase("BoundaryCondition");

  return params;
}

GPUBoundaryCondition::GPUBoundaryCondition(const InputParameters & parameters,
                                           Moose::VarFieldType field_type,
                                           bool nodal)
  : GPUResidualObject(parameters, field_type, nodal), BoundaryRestrictableRequired(this, nodal)
{
}

GPUBoundaryCondition::GPUBoundaryCondition(const GPUBoundaryCondition & object)
  : GPUResidualObject(object), BoundaryRestrictableRequired(object)
{
}
