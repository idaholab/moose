//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PiecewiseMultiInterpolation.h"

// Forward declarations
class GriddedData;

/**
 * Uses GriddedData to define data on a grid,
 * and does linear interpolation on that data to
 * provide function values.
 * Gridded data can be 1D, 2D, 3D or 4D.
 * See GriddedData for examples of file format.
 */
class PiecewiseMulticonstant : public PiecewiseMultiInterpolation
{
public:
  static InputParameters validParams();

  PiecewiseMulticonstant(const InputParameters & parameters);

  using PiecewiseMultiInterpolation::value;
  virtual ADReal value(const ADReal & t, const ADPoint & p) const override;

  virtual RealGradient gradient(Real t, const Point & p) const override;
  virtual Real timeDerivative(Real t, const Point & p) const override;

protected:
  using PiecewiseMultiInterpolation::sample;
  virtual Real sample(const GridPoint & pt) const override;

private:
  /// direction where to look for value if interpolation order is constant
  MultiMooseEnum _direction;
};
