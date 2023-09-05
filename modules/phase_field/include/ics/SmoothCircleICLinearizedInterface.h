//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SmoothCircleIC.h"

/**
 * SmoothCircleICLinearizedInterface creates a circle of a given radius centered at a given point in
 * the domain. If int_width > zero, the border of the circle with smoothly transition from the
 * invalue to the outvalue. It then transforms it with the linearized interface function.
 */
class SmoothCircleICLinearizedInterface : public SmoothCircleIC
{
public:
  static InputParameters validParams();

  SmoothCircleICLinearizedInterface(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);

  const Real _bound;
};
