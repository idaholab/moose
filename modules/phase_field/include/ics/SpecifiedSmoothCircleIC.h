/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPECIFIEDSMOOTHCIRCLEIC_H
#define SPECIFIEDSMOOTHCIRCLEIC_H

#include "SmoothCircleBaseIC.h"

// Forward Declarations
class SpecifiedSmoothCircleIC;

template <>
InputParameters validParams<SpecifiedSmoothCircleIC>();

/**
 * SpecifiedsmoothCircleIC creates multiple SmoothCircles (number = size of x_positions) that are
 *positioned in the
 * set locations with the set radii.  This is adapted from PolySpecifiedSmoothCircleIC from HYRAX by
 *A.M. Jokisaari
 **/
class SpecifiedSmoothCircleIC : public SmoothCircleBaseIC
{
public:
  SpecifiedSmoothCircleIC(const InputParameters & parameters);

protected:
  virtual void computeCircleRadii();
  virtual void computeCircleCenters();

  std::vector<Real> _x_positions;
  std::vector<Real> _y_positions;
  std::vector<Real> _z_positions;
  std::vector<Real> _input_radii;
};

#endif // SPECIFIEDSMOOTHCIRCLEIC_H
