//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdditionalTensorTools.h"

namespace AdditionalTensorTools
{
/// method transposing the first two indeces of a rank 4 tensor
RankFourTensor
R4ijklSwapij(const RankFourTensor & R4)
{
  RankFourTensor res;
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
          res(j, i, k, l) = R4(i, j, k, l);
  return res;
}

/// methods performing unsual tensor multiplications
///@{
RankFourTensor
R2ijR4jklm(const RankTwoTensor & R2, const RankFourTensor & R4)
{
  RankFourTensor res;
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int k = 0; k < 3; k++)
      for (unsigned int l = 0; l < 3; l++)
        for (unsigned int m = 0; m < 3; m++)
        {
          res(i, k, l, m) = 0;
          for (unsigned int j = 0; j < 3; j++)
            res(i, k, l, m) += R2(i, j) * R4(j, k, l, m);
        }
  return res;
}

RankThreeTensor
R4ijklVj(const RankFourTensor & R4, const RealVectorValue & V)
{
  RankThreeTensor res;
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int k = 0; k < 3; k++)
      for (unsigned int l = 0; l < 3; l++)
      {
        res(i, k, l) = 0;
        for (unsigned int j = 0; j < 3; j++)
          res(i, k, l) += R4(i, j, k, l) * V(j);
      }
  return res;
}

RankThreeTensor
R4ijklVi(const RankFourTensor & R4, const RealVectorValue & V)
{
  RankThreeTensor res;
  for (unsigned int j = 0; j < 3; j++)
    for (unsigned int k = 0; k < 3; k++)
      for (unsigned int l = 0; l < 3; l++)
      {
        res(j, k, l) = 0;
        for (unsigned int i = 0; i < 3; i++)
          res(j, k, l) += R4(i, j, k, l) * V(i);
      }
  return res;
}

RankThreeTensor
R2ijR3jkl(const RankTwoTensor & R2, const RankThreeTensor & R3)
{
  RankThreeTensor res;
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
          res(i, k, l) += R2(i, j) * R3(j, k, l);
  return res;
}

RankFourTensor
R4ijklR2jm(const RankFourTensor & R4, const RankTwoTensor & R2)
{
  RankFourTensor res;
  res.zero();
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int k = 0; k < 3; k++)
      for (unsigned int l = 0; l < 3; l++)
        for (unsigned int m = 0; m < 3; m++)
          for (unsigned int j = 0; j < 3; j++)
            res(i, m, k, l) += R4(i, j, k, l) * R2(j, m);
  return res;
}

RankThreeTensor
R2jkVi(const RankTwoTensor & R2, const RealVectorValue & V)
{
  RankThreeTensor res;

  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
        res(i, j, k) = V(i) * R2(j, k);
  return res;
}
///@}
} // namespace AdditionalTensorTools
