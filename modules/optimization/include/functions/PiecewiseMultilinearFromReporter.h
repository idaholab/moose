//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PiecewiseMultiInterpolationFromReporter.h"

/**
 * This is a copy of PiecewiseMultilinear except
 * it is derived from PiecewiseMultiInterpolationFromReporters
 * which manages its own gridded data that it gets fromm a
 * GriddedDataReporter.
 * Uses gridded data to define data on a grid,
 * and does linear interpolation on that data to
 * provide function values.
 * Gridded data can be 1D, 2D, 3D or 4D.
 * See GriddedData for examples of file format.
 */
class PiecewiseMultilinearFromReporter : public PiecewiseMultiInterpolationFromReporter
{

public:
  static InputParameters validParams();

  PiecewiseMultilinearFromReporter(const InputParameters & parameters);

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
