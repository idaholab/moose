/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
