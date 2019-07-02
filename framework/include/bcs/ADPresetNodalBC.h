//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADNodalBC.h"

#define usingPresetNodalBCMembers                                                                  \
  usingNodalBCMembers;                                                                             \
  using ADPresetNodalBC<compute_stage>::computeQpValue

template <ComputeStage>
class ADPresetNodalBC;

declareADValidParams(ADPresetNodalBC);

/**
 * Base class for automatic differentiation nodal BCs that (pre)set the solution
 * vector entries.
 */
template <ComputeStage compute_stage>
class ADPresetNodalBC : public ADNodalBC<compute_stage>
{
public:
  ADPresetNodalBC(const InputParameters & parameters);

  void computeValue(NumericVector<Number> & current_solution);

protected:
  virtual ADReal computeQpResidual() override;
  virtual ADReal computeQpValue() = 0;

  usingNodalBCMembers;
};
