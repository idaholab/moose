//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TabulatedBicubicFluidProperties.h"
#include "BidimensionalInterpolation.h"

/**
 * Exposes TabulatedBicubicFluidProperties::checkNaNs and the specific volume grid for testing
 */
class CheckNaNsTabulatedFluidProperties : public TabulatedBicubicFluidProperties
{
public:
  static InputParameters validParams();

  CheckNaNsTabulatedFluidProperties(const InputParameters & parameters);

  /// Set the specific volume grid and its bounds
  void setSpecificVolumeGrid(const std::vector<Real> & v);

  using TabulatedBicubicFluidProperties::checkNaNs;
};
