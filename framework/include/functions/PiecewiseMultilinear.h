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
class PiecewiseMultilinear : public PiecewiseMultiInterpolation
{
public:
  static InputParameters validParams();

  PiecewiseMultilinear(const InputParameters & parameters);

  virtual RealGradient gradient(Real t, const Point & p) const override;
  virtual Real timeDerivative(Real t, const Point & p) const override;

protected:
  virtual Real sample(const GridPoint & pt) const override;
  virtual ADReal sample(const ADGridPoint & pt) const override;

  const Real _epsilon;

private:
  template <bool is_ad>
  MooseADWrapper<Real, is_ad> sampleInternal(const MooseADWrapper<GridPoint, is_ad> pt) const;
};
