//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOMPUTEINCREMENTALSMALLSTRAIN_H
#define ADCOMPUTEINCREMENTALSMALLSTRAIN_H

#include "ADComputeIncrementalStrainBase.h"

#define usingComputeIncrementalSmallStrainMembers                                                  \
  usingComputeIncrementalStrainBaseMembers;                                                        \
  using ADComputeIncrementalSmallStrain<compute_stage>::computeTotalStrainIncrement

template <ComputeStage>
class ADComputeIncrementalSmallStrain;

declareADValidParams(ADComputeIncrementalSmallStrain);

/**
 * ADComputeIncrementalSmallStrain defines a strain increment and rotation increment (=1), for small
 * strains.
 */
template <ComputeStage compute_stage>
class ADComputeIncrementalSmallStrain : public ADComputeIncrementalStrainBase<compute_stage>
{
public:
  ADComputeIncrementalSmallStrain(const InputParameters & parameters);

  virtual void computeProperties() override;

protected:
  /**
   * Computes the current and old deformation gradients and passes back the
   * total strain increment tensor.
   */
  virtual void computeTotalStrainIncrement(ADRankTwoTensor & total_strain_increment);

  usingComputeIncrementalStrainBaseMembers;
};

#endif // ADCOMPUTEINCREMENTALSMALLSTRAIN_H
