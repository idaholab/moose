//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
class EnrichmentFunctionCalculation
{
public:
  EnrichmentFunctionCalculation(const CrackFrontDefinition * crack_front_definition);

  /** calculate the enrichment function values at point
   * @return the closest crack front index
   */
  virtual unsigned int crackTipEnrichementFunctionAtPoint(const Point & point,
                                                          std::vector<Real> & B);

  /** calculate the enrichment function derivatives at point
   * @return the closest crack front index
   */
  virtual unsigned int
  crackTipEnrichementFunctionDerivativeAtPoint(const Point & point,
                                               std::vector<RealVectorValue> & dB);

  /** rotate a vector from crack front coordinate to global cooridate
   * @param rotated_vector rotated vector
   */
  void rotateFromCrackFrontCoordsToGlobal(const RealVectorValue & vector,
                                          RealVectorValue & rotated_vector,
                                          const unsigned int point_index);

private:
  const CrackFrontDefinition & _crack_front_definition;
  Real _r;
  Real _theta;
};
