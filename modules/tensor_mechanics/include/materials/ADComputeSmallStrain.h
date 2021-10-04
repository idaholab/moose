//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeStrainBase.h"

#define usingComputeSmallStrainMembers usingComputeStrainBaseMembers

/**
 * ADComputeSmallStrain defines a strain tensor, assuming small strains.
 */
template <typename R2>
class ADComputeSmallStrainTempl : public ADComputeStrainBaseTempl<R2>
{
public:
  static InputParameters validParams();

  ADComputeSmallStrainTempl(const InputParameters & parameters);

  virtual void computeProperties() override;

  usingComputeStrainBaseMembers;
};

typedef ADComputeSmallStrainTempl<RankTwoTensor> ADComputeSmallStrain;
typedef ADComputeSmallStrainTempl<SymmetricRankTwoTensor> ADSymmetricSmallStrain;
