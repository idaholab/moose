//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
