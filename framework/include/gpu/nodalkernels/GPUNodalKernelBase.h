//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUResidualObject.h"

#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

class GPUNodalKernelBase : public GPUResidualObject,
                           public BlockRestrictable,
                           public BoundaryRestrictable,
                           public CoupleableMooseVariableDependencyIntermediateInterface
{
public:
  static InputParameters validParams();

  GPUNodalKernelBase(const InputParameters & parameters, Moose::VarFieldType field_type);
  GPUNodalKernelBase(const GPUNodalKernelBase & object);
};

#define usingGPUNodalKernelBaseMembers                                                             \
  usingGPUResidualObjectMembers;                                                                   \
                                                                                                   \
protected:                                                                                         \
  using GPUNodalKernelBase::blockNodeID;                                                           \
  using GPUNodalKernelBase::boundaryNodeID;
