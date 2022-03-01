//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "LineMaterialSamplerBase.h"

/**
 * This class samples Real material properties for the integration points
 * in all elements that are intersected by a user-defined line.
 */
class LineMaterialRealSampler : public LineMaterialSamplerBase<Real>
{
public:
  static InputParameters validParams();

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
