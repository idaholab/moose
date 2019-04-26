//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADSplitCHBase.h"

#define usingSplitCHCResMembers                                                                    \
  usingSplitCHBaseMembers;                                                                         \
  using ADSplitCHCRes<compute_stage>::_kappa;                                                      \
  using ADSplitCHCRes<compute_stage>::_w

// Forward Declarations
template <ComputeStage>
class ADSplitCHCRes;

declareADValidParams(ADSplitCHCRes);

/**
 * The pair, ADSplitCHCRes and ADSplitCHWRes, splits the Cahn-Hilliard equation
 * by replacing chemical potential with 'w'.
 */
template <ComputeStage compute_stage>
class ADSplitCHCRes : public ADSplitCHBase<compute_stage>
{
public:
  ADSplitCHCRes(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  const ADMaterialProperty(Real) & _kappa;
  const ADVariableValue & _w;

  usingSplitCHBaseMembers;
};

