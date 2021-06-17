//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SmoothCircleBaseIC.h"

/**
 * SpecifiedsmoothCircleIC creates multiple SmoothCircles (number = size of x_positions)
 * that are positioned in the set locations with the set radii.
 * This is adapted from PolySpecifiedSmoothCircleIC from HYRAX by A.M. Jokisaari.
 */
class SpecifiedSmoothCircleIC : public SmoothCircleBaseIC
{
public:
  static InputParameters validParams();

  SpecifiedSmoothCircleIC(const InputParameters & parameters);

protected:
  virtual void computeCircleRadii();
  virtual void computeCircleCenters();

  std::vector<Real> _x_positions;
  std::vector<Real> _y_positions;
  std::vector<Real> _z_positions;
  std::vector<Real> _input_radii;
};
