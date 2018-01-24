/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CLOSEPACKIC_H
#define CLOSEPACKIC_H

// MOOSE includes
#include "SmoothCircleBaseIC.h"

// Forward declarations
class ClosePackIC;

template <>
InputParameters validParams<ClosePackIC>();

/**
 * An InitialCondition for initializing phase variable in close packed circles/spheres pattern
 */
class ClosePackIC : public SmoothCircleBaseIC
{
public:
  ClosePackIC(const InputParameters & parameters);

protected:
  /// The radius are populated in the computeCircleCenters
  virtual void computeCircleRadii() {}

  /// Compute the close packed centers and radii
  virtual void computeCircleCenters();

  /// User-supplied circle radius
  const Real _radius;
};

#endif // CLOSEPACKIC_H
