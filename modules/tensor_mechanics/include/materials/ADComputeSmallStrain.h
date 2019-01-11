//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOMPUTESMALLSTRAIN_H
#define ADCOMPUTESMALLSTRAIN_H

#include "ADComputeStrainBase.h"

#define usingComputeSmallStrainMembers usingComputeStrainBaseMembers

template <ComputeStage>
class ADComputeSmallStrain;

declareADValidParams(ADComputeSmallStrain);

/**
 * ADComputeSmallStrain defines a strain tensor, assuming small strains.
 */
template <ComputeStage compute_stage>
class ADComputeSmallStrain : public ADComputeStrainBase<compute_stage>
{
public:
  ADComputeSmallStrain(const InputParameters & parameters);

  virtual void computeProperties() override;

protected:
  usingComputeStrainBaseMembers;
};

#endif // ADCOMPUTESMALLSTRAIN_H
