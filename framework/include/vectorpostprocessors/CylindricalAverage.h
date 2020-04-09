//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SpatialAverageBase.h"

/**
 * Compute a cylindrical average of a variableas a function of radius throughout the
 * simulation domain.
 */
class CylindricalAverage : public SpatialAverageBase
{
public:
  static InputParameters validParams();

  CylindricalAverage(const InputParameters & parameters);

protected:
  /// compute the distance of the current quadarature point for binning
  virtual Real computeDistance() override;

  /// vector along cylinder axis
  const Point _cyl_axis;

  /// axis norm
  const Real _cyl_axis_norm;
};
