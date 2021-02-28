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

namespace AdditionalTensorTools
{
/// method transposing the first two indeces of a rank 4 tensor
RankFourTensor R4ijklSwapij(const RankFourTensor & R4);

/// methods performing unsual tensor multiplications
///@{
RankFourTensor R2ijR4jklm(const RankTwoTensor & R2, const RankFourTensor & R4);
RankThreeTensor R4ijklVj(const RankFourTensor & R4, const RealVectorValue & V);

RankThreeTensor R4ijklVi(const RankFourTensor & R4, const RealVectorValue & V);

RankThreeTensor R2ijR3jkl(const RankTwoTensor & R2, const RankThreeTensor & R3);
RankFourTensor R4ijklR2jm(const RankFourTensor & R4, const RankTwoTensor & R2);

RankThreeTensor R2jkVi(const RankTwoTensor & R2, const RealVectorValue & V);
///@}
} // namespace AdditionalTensorTools
