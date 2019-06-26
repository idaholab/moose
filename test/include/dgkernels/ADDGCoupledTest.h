//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDGKernel.h"

// Forward Declarations
template <ComputeStage>
class ADDGCoupledTest;

declareADValidParams(ADDGCoupledTest);

/**
 * This class is only currently used to test whether we can request neighbor AD calculations and not
 * have anything go horribly wrong
 */
template <ComputeStage compute_stage>
class ADDGCoupledTest : public ADDGKernel<compute_stage>
{
public:
  ADDGCoupledTest(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  MooseVariable & _v_var;
  const ADVariableValue & _v;
  const ADVariableValue & _v_neighbor;

  usingDGKernelMembers;
};

