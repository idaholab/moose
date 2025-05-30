//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PiecewiseLinearBase.h"

/**
 * Function class that reads in a list of (x,y) value pairs representing a point-wise defined
 * function similar to PiecewiseLinear. In addition this Function object performs a point reduction
 * of the tabulated data upon initialization resulting in the evaluation of a simplified function
 * with fewer data points.
 */
class CoarsenedPiecewiseLinear : public PiecewiseLinearBase
{
public:
  static InputParameters validParams();

  CoarsenedPiecewiseLinear(const InputParameters & parameters);

  /// Needed to process data loaded from user objects that are not available at construction
  void initialSetup() override;

  /// Builds the coarse linear interpolation from the fine raw data
  void buildCoarsenedGrid();
};
