//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTISMOOTHCIRCLEIC_H
#define MULTISMOOTHCIRCLEIC_H

#include "SmoothCircleBaseIC.h"

// Forward Declarations
class MultiSmoothCircleIC;

template <>
InputParameters validParams<MultiSmoothCircleIC>();

/**
 * MultismoothCircleIC creates multiple SmoothCircles (number = numbub) that are randomly
 * positioned around the domain, with a minimum spacing equal to bubspac
 */
class MultiSmoothCircleIC : public SmoothCircleBaseIC
{
public:
  MultiSmoothCircleIC(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual void computeCircleRadii();
  virtual void computeCircleCenters();

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

#endif // MULTISMOOTHCIRCLEIC_H
