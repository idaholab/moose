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
 * This is a copy of PiecewiseMulticonstant except
 * it is derived from PiecewiseMultiInterpolationFromReporters
 * WHICH gets manages its own gridded data that it gets form a
 * GriddedDataReporter
 * Uses GriddedData to define data on a grid,
 * and does linear interpolation on that data to
 * provide function values.
 * Gridded data can be 1D, 2D, 3D or 4D.
 * See GriddedData for examples of file format.
 */
class PiecewiseMulticonstantFromReporter : public PiecewiseMultiInterpolationFromReporter
{
public:
  static InputParameters validParams();

  PiecewiseMulticonstantFromReporter(const InputParameters & parameters);

  virtual void initialSetup() override;

  using PiecewiseMultiInterpolationFromReporter::value;
  virtual ADReal value(const ADReal & t, const ADPoint & p) const override;

  virtual RealGradient gradient(Real t, const Point & p) const override;
  virtual Real timeDerivative(Real t, const Point & p) const override;

protected:
  using PiecewiseMultiInterpolationFromReporter::sample;
  virtual Real sample(const GridPoint & pt) const override;

private:
  /// direction where to look for value if interpolation order is constant
  MultiMooseEnum _direction;
};
