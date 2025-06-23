//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUResidualObject.h"

#include "BlockRestrictable.h"
#include "MaterialPropertyInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

class GPUKernelBase : public GPUResidualObject,
                      public BlockRestrictable,
                      public CoupleableMooseVariableDependencyIntermediateInterface,
                      public MaterialPropertyInterface
{
public:
  static InputParameters validParams();

  GPUKernelBase(const InputParameters & parameters, Moose::VarFieldType field_type);
  GPUKernelBase(const GPUKernelBase & object);

protected:
  // Sets the variables this object depend on
  void setVariableDependency();
  // Sets the quadrature projection status flags for the variables, tags, and subdomains covered by
  // this object
  void setProjectionFlags();
};

#define usingGPUKernelBaseMembers                                                                  \
  usingGPUResidualObjectMembers;                                                                   \
                                                                                                   \
protected:                                                                                         \
  using GPUKernelBase::blockElementID;
