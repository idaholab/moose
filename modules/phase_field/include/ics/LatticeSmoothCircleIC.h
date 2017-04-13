/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LATTICESMOOTHCIRCLEIC_H
#define LATTICESMOOTHCIRCLEIC_H

#include "SmoothCircleBaseIC.h"

// Forward Declarations
class LatticeSmoothCircleIC;

template <>
InputParameters validParams<LatticeSmoothCircleIC>();

/**
 * LatticeSmoothcircleIC creates a lattice of smoothcircles as an initial condition.
 * They are either directly on the lattice or randomly perturbed from the lattice.
 */
class LatticeSmoothCircleIC : public SmoothCircleBaseIC
{
public:
  LatticeSmoothCircleIC(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual void computeCircleRadii();
  virtual void computeCircleCenters();

  Real _lattice_variation;
  std::vector<unsigned int> _circles_per_side;
  unsigned int _numbub;

  Real _radius;
  Real _radius_variation;
  MooseEnum _radius_variation_type;

  bool _avoid_bounds;

  Point _bottom_left;
  Point _top_right;
  Point _range;
};

#endif // LATTICESMOOTHCIRCLEIC_H
