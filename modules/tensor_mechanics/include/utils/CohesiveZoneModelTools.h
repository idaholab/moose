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
#include "libmesh/vector_value.h"
#include "MooseTypes.h"
#include "RotationMatrix.h"

namespace CohesiveZoneModelTools
{
template <bool is_ad = false>
GenericRealVectorValue<is_ad>
computedVnormdV(const GenericRealVectorValue<is_ad> & V)
{
  return V / V.norm();
}

/// compute the derivative of the rotation matrix, R=FU^-1, using Chen and Wheeler 1993
template <bool is_ad = false>
GenericRankFourTensor<is_ad>
computedRdF(const GenericRankTwoTensor<is_ad> & R, const GenericRankTwoTensor<is_ad> & U)
{
  const auto Uhat = U.trace() * RankTwoTensor::Identity() - U;
  unsigned int k, l, m, n, p, q;
  const auto Uhat_det = Uhat.det();

  GenericRankFourTensor<is_ad> dR_dF;
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

/// compute the derivative of F^-1 w.r.t. to F
RankFourTensor computedFinversedF(const RankTwoTensor & F_inv);

/// compute the area ratio betweeen the deformed and undeformed configuration, and its derivatives w.r.t. the deformation gradient, F
///@{
template <bool is_ad = false>
GenericReal<is_ad>
computeAreaRatio(const RankTwoTensor & FinvT, const Real & J, const RealVectorValue & N)
{
  return J * (FinvT * N).norm();
}

template <bool is_ad = false>
GenericRankTwoTensor<is_ad>
computeDAreaRatioDF(const GenericRankTwoTensor<is_ad> & FinvT,
                    const GenericRealVectorValue<is_ad> & N,
                    const GenericReal<is_ad> & J,
                    const GenericRankFourTensor<is_ad> & DFinv_DF)
{
  const auto Fitr_N = FinvT * N;
  const auto Fitr_N_norm = Fitr_N.norm();
  GenericRankTwoTensor<is_ad> R2temp;
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
///@}

/// compute the normal componets of a vector
template <bool is_ad = false>
GenericRealVectorValue<is_ad>
computeNormalComponents(const GenericRealVectorValue<is_ad> & normal,
                        const GenericRealVectorValue<is_ad> & vector)
{
  return (normal * vector) * normal;
}

/// compute the tangent componets of a vector
template <bool is_ad = false>
GenericRealVectorValue<is_ad>
computeTangentComponents(const GenericRealVectorValue<is_ad> & normal,
                         const GenericRealVectorValue<is_ad> & vector)
{
  return vector - computeNormalComponents<is_ad>(normal, vector);
}

/// compute the czm reference rotation based on the normal in the undeformed configuration and the mesh dimension
template <bool is_ad = false>
GenericRankTwoTensor<is_ad>
computeReferenceRotation(const GenericRealVectorValue<is_ad> & normal,
                         const unsigned int mesh_dimension)
{
  GenericRankTwoTensor<is_ad> rot;
  switch (mesh_dimension)
  {
    case 3:
      rot = RotationMatrix::rotVec1ToVec2<is_ad>(GenericRealVectorValue<false>(1, 0, 0), normal);
      break;
    case 2:
      rot = RotationMatrix::rotVec2DToX<is_ad>(normal).transpose();
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
