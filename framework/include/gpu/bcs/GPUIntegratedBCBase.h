//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUBoundaryCondition.h"

#include "MaterialPropertyInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

class GPUIntegratedBCBase : public GPUBoundaryCondition,
                            public CoupleableMooseVariableDependencyIntermediateInterface,
                            public MaterialPropertyInterface
{
public:
  static InputParameters validParams();

  GPUIntegratedBCBase(const InputParameters & parameters, Moose::VarFieldType field_type);
  GPUIntegratedBCBase(const GPUIntegratedBCBase & object);
};

#define usingGPUIntegratedBCBaseMembers usingGPUBoundaryConditionMembers;
