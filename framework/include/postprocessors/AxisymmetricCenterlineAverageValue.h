//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideAverageValue.h"

/**
 * This postprocessor computes a line integral of the specified variable
 * along the centerline of an axisymmetric domain.
 */
class AxisymmetricCenterlineAverageValue : public SideAverageValue
{
public:
  static InputParameters validParams();

  AxisymmetricCenterlineAverageValue(const InputParameters & parameters);

protected:
  virtual Real computeIntegral() override;
  virtual Real volume() override;
  Real _volume;
};
