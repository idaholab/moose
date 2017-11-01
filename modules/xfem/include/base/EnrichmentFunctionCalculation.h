/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef ENRICHMENTFUNCTIONCALCULATION_H
#define ENRICHMENTFUNCTIONCALCULATION_H

#include "CrackFrontDefinition.h"

class EnrichmentFunctionCalculation;

template <>
InputParameters validParams<EnrichmentFunctionCalculation>();

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
  const CrackFrontDefinition * _crack_front_definition;
  Real _r;
  Real _theta;
};

#endif // ENRICHMENTFUNCTIONCALCULATION_H
