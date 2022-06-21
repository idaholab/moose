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

RealVectorValue
computedVnormdV(const RealVectorValue & V)
{
  return V / V.norm();
}

RankFourTensor
computedRdF(const RankTwoTensor & R, const RankTwoTensor & U)
{
  const RankTwoTensor Uhat = U.trace() * RankTwoTensor::Identity() - U;
  unsigned int k, l, m, n, p, q;
  const Real Uhat_det = Uhat.det();

  RankFourTensor dR_dF;
  for (k = 0; k < 3; k++)
    for (l = 0; l < 3; l++)
      for (m = 0; m < 3; m++)
        for (n = 0; n < 3; n++)
        {
          dR_dF(k, l, m, n) = 0.;
          for (p = 0; p < 3; p++)
            for (q = 0; q < 3; q++)
              dR_dF(k, l, m, n) +=
                  R(k, p) * (Uhat(p, q) * R(m, q) * Uhat(n, l) - Uhat(p, n) * R(m, q) * Uhat(q, l));

          dR_dF(k, l, m, n) /= Uhat_det;
        }

  return dR_dF;
}

RankFourTensor
computedFinversedF(const RankTwoTensor & F_inv)
{
  usingTensorIndices(i_, j_, k_, l_);
  return -F_inv.times<i_, k_, j_, l_>(F_inv.transpose());
}

Real
computeAreaRatio(const RankTwoTensor & FinvT, const Real & J, const RealVectorValue & N)
{
  return J * (FinvT * N).norm();
}

RankTwoTensor
computeDAreaRatioDF(const RankTwoTensor & FinvT,
                    const RealVectorValue & N,
                    const Real & J,
                    const RankFourTensor & DFinv_DF)
{
  const RealVectorValue Fitr_N = FinvT * N;
  const Real Fitr_N_norm = Fitr_N.norm();
  RankTwoTensor R2temp;
  for (unsigned int l = 0; l < 3; l++)
    for (unsigned int m = 0; m < 3; m++)
    {
      R2temp(l, m) = 0;
      for (unsigned int i = 0; i < 3; i++)
        for (unsigned int j = 0; j < 3; j++)
          R2temp(l, m) += Fitr_N(i) * DFinv_DF(j, i, l, m) * N(j);

      R2temp(l, m) *= J / Fitr_N_norm;
    }

  return J * FinvT * Fitr_N_norm + R2temp;
}

RealVectorValue
computeNormalComponents(const RealVectorValue & normal, const RealVectorValue & vector)
{
  return (normal * vector) * normal;
}

RealVectorValue
computeTangentComponents(const RealVectorValue & normal, const RealVectorValue & vector)
{
  return vector - computeNormalComponents(normal, vector);
}

RankTwoTensor
computeReferenceRotation(const RealVectorValue & normal, const unsigned int mesh_dimension)
{
  RankTwoTensor rot;
  switch (mesh_dimension)
  {
    case 3:
      rot = RotationMatrix::rotVec1ToVec2(RealVectorValue(1, 0, 0), normal);
      break;
    case 2:
      rot = RotationMatrix::rotVec2DToX(normal).transpose();
      break;
    case 1:
      rot = RankTwoTensor::Identity();
      break;
    default:
      mooseError("computeReferenceRotation: mesh_dimension value should be 1, 2 or, 3. You "
                 "provided " +
                 std::to_string(mesh_dimension));
  }
  return rot;
}

} // namespace CohesiveZoneModelTools
