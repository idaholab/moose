//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrackFrontDefinition.h"

/**
 * Perform calculation of enrichment function values and derivatives.
 */
class EnrichFunctionUtility
{
public:
  EnrichFunctionUtility() {}

  /** calculate the enrichment function values at point
   * @return the closest crack front index
   */
  static unsigned int crackTipEnrichementFunctionAtPoint(const CrackFrontDefinition * cfd,
                                                         const Point & point,
                                                         std::vector<Real> & B);

  /** calculate the enrichment function derivatives at point
   * @return the closest crack front index
   */
  static unsigned int crackTipEnrichementFunctionDerivativeAtPoint(
      const CrackFrontDefinition * cfd, const Point & point, std::vector<RealVectorValue> & dB);

  /** rotate a vector from crack front coordinate to global cooridate
   * @param rotated_vector rotated vector
   */
  static void rotateFromCrackFrontCoordsToGlobal(const CrackFrontDefinition * cfd,
                                                 const RealVectorValue & vector,
                                                 RealVectorValue & rotated_vector,
                                                 const unsigned int point_index);
};
