//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADSPLITCHBASE_H
#define ADSPLITCHBASE_H

#include "ADKernel.h"

#define usingSplitCHBaseMembers                                                                    \
  usingKernelMembers;                                                                              \
  using ADSplitCHBase<compute_stage>::computeQpResidual;                                           \
  using ADSplitCHBase<compute_stage>::computeDFDC

// Forward Declarations
template <ComputeStage>
class ADSplitCHBase;

declareADValidParams(ADSplitCHBase);

/**
 * The pair, ADSplitCHCRes and ADSplitCHWRes, splits the Cahn-Hilliard equation
 * by replacing chemical potential with 'w'.
 */
template <ComputeStage compute_stage>
class ADSplitCHBase : public ADKernel<compute_stage>
{
public:
  ADSplitCHBase(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();
  virtual ADReal computeDFDC();

  usingKernelMembers;
};

#endif // ADSPLITCHBASE_H
