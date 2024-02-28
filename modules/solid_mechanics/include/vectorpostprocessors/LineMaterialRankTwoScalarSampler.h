//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LineMaterialSamplerBase.h"
#include "RankTwoTensor.h"

// Forward Declarations

/**
 * This class samples RankTwoTensor material properties for the integration points
 * in all elements that are intersected by a user-defined line.  It provides
 * access to the full set of options for reducing the RankTwoTensor to a scalar
 */
class LineMaterialRankTwoScalarSampler : public LineMaterialSamplerBase<RankTwoTensor>
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * Sets up variables for output based on the properties to be output
   * @param parameters The input parameters
   */
  LineMaterialRankTwoScalarSampler(const InputParameters & parameters);

  /**
   * Reduce the RankTwoTensor material property to a scalar for output
   * Call through to RankTwoScalarTools::getQuantity to access the full set of options for reducing
   * the RankTwoTensor to a scalar quantity
   * @param property The material property
   * @param curr_point The point corresponding to this material property
   * @return A scalar value from this material property to be output
   */
  virtual Real getScalarFromProperty(const RankTwoTensor & property, const Point & curr_point);

protected:
  MooseEnum _scalar_type;

  const Point _point1;
  const Point _point2;
  Point _direction;
};
