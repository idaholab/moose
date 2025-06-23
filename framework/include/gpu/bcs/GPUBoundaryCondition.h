//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUResidualObject.h"

#include "BoundaryRestrictableRequired.h"

#pragma once

class GPUBoundaryCondition : public GPUResidualObject, public BoundaryRestrictableRequired
{
public:
  static InputParameters validParams();

  GPUBoundaryCondition(const InputParameters & parameters,
                       Moose::VarFieldType field_type,
                       bool nodal);
  GPUBoundaryCondition(const GPUBoundaryCondition & object);
};

#define usingGPUBoundaryConditionMembers                                                           \
  usingGPUResidualObjectMembers;                                                                   \
                                                                                                   \
protected:                                                                                         \
  using GPUBoundaryCondition::boundaryElementSideID;                                               \
  using GPUBoundaryCondition::boundaryNodeID;
