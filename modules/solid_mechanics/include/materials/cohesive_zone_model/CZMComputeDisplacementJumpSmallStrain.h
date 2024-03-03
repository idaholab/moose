//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMComputeDisplacementJumpBase.h"

/**
 * Compute the interface displacement jump across a cohesive zone under the small strain
 * assumption
 */
template <bool is_ad>
class CZMComputeDisplacementJumpSmallStrainTempl : public CZMComputeDisplacementJumpBase<is_ad>
{
public:
  static InputParameters validParams();
  CZMComputeDisplacementJumpSmallStrainTempl(const InputParameters & parameters);

protected:
  /// compute the total displacement jump in interface coordinates
  void computeLocalDisplacementJump() override;

  usingCZMComputeDisplacementJumpBaseMembers;
};

typedef CZMComputeDisplacementJumpSmallStrainTempl<false> CZMComputeDisplacementJumpSmallStrain;
typedef CZMComputeDisplacementJumpSmallStrainTempl<true> ADCZMComputeDisplacementJumpSmallStrain;
