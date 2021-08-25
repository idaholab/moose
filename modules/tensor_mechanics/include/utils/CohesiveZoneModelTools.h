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

namespace CohesiveZoneModelTools
{

RealVectorValue computedVnormdV(const RealVectorValue & V);

/// compute the derivative of the rotation matrix, R=FU^-1, using Chen and Wheeler 1993
RankFourTensor computedRdF(const RankTwoTensor & R, const RankTwoTensor & U);

/// compute the derivative of F^-1 w.r.t. to F
RankFourTensor computedFinversedF(const RankTwoTensor & F_inv);

/// compute the area ratio betweeen the deformed and undeformed configuration, and its derivatives w.r.t. the deformation gradient, F
///@{
Real computeAreaRatio(const RankTwoTensor & FinvT, const Real & J, const RealVectorValue & N);
RankTwoTensor computeDAreaRatioDF(const RankTwoTensor & FinvT,
                                  const RealVectorValue & N,
                                  const Real & J,
                                  const RankFourTensor & DFinv_DF);
///@}

/// compute the normal componets of a vector
RealVectorValue computeNormalComponents(const RealVectorValue & normal,
                                        const RealVectorValue & vector);

/// compute the tangent componets of a vector
RealVectorValue computeTangentComponents(const RealVectorValue & normal,
                                         const RealVectorValue & vector);

/// compute the czm reference rotation based on the normal in the undeformed configuration and the mesh dimension
RankTwoTensor computeReferenceRotation(const RealVectorValue & normal,
                                       const unsigned int mesh_dimension);

} // namespace CohesiveZoneModelTools
