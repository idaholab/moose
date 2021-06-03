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
 * Compute the interface displacement jump accross a cohesive zone under the small strain
 * assumption
 */
class CZMComputeDisplacementJumpSmallStrain : public CZMComputeDisplacementJumpBase
{
public:
  static InputParameters validParams();
  CZMComputeDisplacementJumpSmallStrain(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;

  /// compute the total displacement jump in interface coordinates
  void computeLocalDisplacementJump() override;
};
