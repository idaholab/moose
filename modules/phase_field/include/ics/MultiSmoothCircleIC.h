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
 * MultismoothCircleIC creates multiple SmoothCircles (number = numbub) that are randomly
 * positioned around the domain with a minimum spacing equal to bubspac. The system attempts to
 * randomly place bubbles in the domain until the desired number of distinct bubbles are placed.
 * If the number of attempts exceeds "max_tries", a mooseError will be thrown and the program will
 * terminate.
 */
class MultiSmoothCircleIC : public SmoothCircleBaseIC
{
public:
  static InputParameters validParams();

  MultiSmoothCircleIC(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual void computeCircleRadii() override;
  virtual void computeCircleCenters() override;

  const unsigned int _numbub;
  const Real _bubspac;

  const unsigned int _max_num_tries;

  const Real _radius;
  const Real _radius_variation;
  const MooseEnum _radius_variation_type;

  Point _bottom_left;
  Point _top_right;
  Point _range;
};
