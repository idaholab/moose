//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SCZMComputeDisplacementJumpBase.h"

/**
 * Compute the interface displacement jump across a cohesive zone under the small strain
 * assumption
 */
template <bool is_ad>
class SCZMComputeDisplacementJumpSmallStrainTempl : public SCZMComputeDisplacementJumpBase<is_ad>
{
public:
  static InputParameters validParams();
  SCZMComputeDisplacementJumpSmallStrainTempl(const InputParameters & parameters);

protected:
  /// compute the total displacement jump in interface coordinates
  void computeLocalDisplacementJump() override;

  usingSCZMComputeDisplacementJumpBaseMembers;
};

typedef SCZMComputeDisplacementJumpSmallStrainTempl<false> SCZMComputeDisplacementJumpSmallStrain;
typedef SCZMComputeDisplacementJumpSmallStrainTempl<true> ADSCZMComputeDisplacementJumpSmallStrain;
