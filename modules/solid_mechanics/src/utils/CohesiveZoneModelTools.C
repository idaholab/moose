//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CohesiveZoneModelTools.h"
#include "RotationMatrix.h"

namespace CohesiveZoneModelTools
{
RankFourTensor
computedFinversedF(const RankTwoTensor & F_inv)
{
  usingTensorIndices(i_, j_, k_, l_);
  return -F_inv.times<i_, k_, j_, l_>(F_inv.transpose());
}
} // namespace CohesiveZoneModelTools
