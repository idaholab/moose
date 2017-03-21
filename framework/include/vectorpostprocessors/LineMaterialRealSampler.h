/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef LINEMATERIALREALSAMPLER_H
#define LINEMATERIALREALSAMPLER_H

// MOOSE includes
#include "LineMaterialSamplerBase.h"

// Forward Declarations
class LineMaterialRealSampler;

template <>
InputParameters validParams<LineMaterialRealSampler>();

/**
 * This class samples Real material properties for the integration points
 * in all elements that are intersected by a user-defined line.
 */
class LineMaterialRealSampler : public LineMaterialSamplerBase<Real>
{
public:
  /**
   * Class constructor
   * Sets up variables for output based on the properties to be output
   * @param parameters The input parameters
   */
  LineMaterialRealSampler(const InputParameters & parameters);

  /**
   * Reduce the material property to a scalar for output
   * In this case, the material property is a Real already, so just return it.
   * @param property The material property
   * @param curr_point The point corresponding to this material property
   * @return A scalar value from this material property to be output
   */
  virtual Real getScalarFromProperty(const Real & property, const Point & curr_point) override;
};

#endif
