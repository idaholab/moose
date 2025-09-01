//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EnrichFunctionUtility.h"

/**
 * Perform calculation of enrichment function values and derivatives.
 */

/** calculate the enrichment function values at point
 * @return the closest crack front index
 */
unsigned int
EnrichFunctionUtility::crackTipEnrichementFunctionAtPoint(const CrackFrontDefinition * cfd,
                                                          const Point & point,
                                                          std::vector<Real> & B)
{
  Real r, theta;
  unsigned int crack_front_point_index = cfd->calculateRThetaToCrackFront(point, r, theta);

  if (MooseUtils::absoluteFuzzyEqual(r, 0.0))
    mooseError("EnrichmentFunctionCalculation: the distance between a point and the crack "
               "tip/front is zero.");

  Real st = std::sin(theta);
  Real st2 = std::sin(theta / 2.0);
  Real ct2 = std::cos(theta / 2.0);
  Real sr = std::sqrt(r); // std::pow(r, 1.0 / 8); //

  B[0] = sr * st2;
  B[1] = sr * ct2;
  B[2] = sr * st2 * st;
  B[3] = sr * ct2 * st;

  return crack_front_point_index;
}

/** calculate the enrichment function derivatives at point
 * @return the closest crack front index
 */
unsigned int
EnrichFunctionUtility::crackTipEnrichementFunctionDerivativeAtPoint(
    const CrackFrontDefinition * cfd, const Point & point, std::vector<RealVectorValue> & dB)
{
  Real r, theta;
  unsigned int crack_front_point_index = cfd->calculateRThetaToCrackFront(point, r, theta);

  if (MooseUtils::absoluteFuzzyEqual(r, 0.0))
    mooseError("EnrichmentFunctionCalculation: the distance between a point and the crack "
               "tip/front is zero.");

  Real st = std::sin(theta);
  Real ct = std::cos(theta);
  Real st2 = std::sin(theta / 2.0);
  Real ct2 = std::cos(theta / 2.0);
  Real st15 = std::sin(1.5 * theta);
  Real ct15 = std::cos(1.5 * theta);
  Real sr = std::sqrt(r); // std::pow(r, 1.0 / 8); //

  dB[0](0) = -0.5 / sr * st2;
  dB[0](1) = 0.5 / sr * ct2;
  dB[0](2) = 0.0;
  dB[1](0) = 0.5 / sr * ct2;
  dB[1](1) = 0.5 / sr * st2;
  dB[1](2) = 0.0;
  dB[2](0) = -0.5 / sr * st15 * st;
  dB[2](1) = 0.5 / sr * (st2 + st15 * ct);
  dB[2](2) = 0.0;
  dB[3](0) = -0.5 / sr * ct15 * st;
  dB[3](1) = 0.5 / sr * (ct2 + ct15 * ct);
  dB[3](2) = 0.0;

  return crack_front_point_index;
}

/** rotate a vector from crack front coordinate to global cooridate
 * @param rotated_vector rotated vector
 */
void
EnrichFunctionUtility::rotateFromCrackFrontCoordsToGlobal(const CrackFrontDefinition * cfd,
                                                          const RealVectorValue & vector,
                                                          RealVectorValue & rotated_vector,
                                                          const unsigned int point_index)
{
  rotated_vector = cfd->rotateFromCrackFrontCoordsToGlobal(vector, point_index);
}
