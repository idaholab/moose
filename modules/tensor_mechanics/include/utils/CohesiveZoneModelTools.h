//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RankFourTensor.h"
#include "RankTwoTensor.h"
#include "Assembly.h"

namespace CohesiveZoneModelTools
{

RealVectorValue computedVnormdV(const RealVectorValue & V);

RankFourTensor computedRdF(const RankTwoTensor & R, const RankTwoTensor & U);

RankFourTensor computedFinversedF(const RankTwoTensor & F_inv);

RankTwoTensor computeVelocityGradientLinearApprox(const RankTwoTensor & F,
                                                  const RankTwoTensor & F_old);

RankFourTensor computeDL_DF(const RankFourTensor & DFinv_DF, const RankTwoTensor & F_old);

RankTwoTensor computeDtraceL_DF(const RankFourTensor & DL_DF);

Real computeAreaRatio(const RankTwoTensor & FinvT, const Real & J, const RealVectorValue & N);

RankTwoTensor computeDAreaRatioDF(const RankTwoTensor & FinvT,
                                  const RealVectorValue & N,
                                  const Real & J,
                                  const RankFourTensor & DFinv_DF);

Real
computeAreaIncrementRate(const Real Ltrace, const RankTwoTensor & L, const RealVectorValue & n);

RankTwoTensor computeDAreaIncrementRateDF(const RankTwoTensor & L,
                                          const RankTwoTensor & DLtrace_DF,
                                          const RankFourTensor & DL_DF,
                                          const RealVectorValue & N,
                                          const RankTwoTensor & R,
                                          const RankFourTensor & DR_DF);
} // namespace CohesiveZoneModelTools
