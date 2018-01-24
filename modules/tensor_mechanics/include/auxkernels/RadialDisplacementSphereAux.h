/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RADIALDISPLACEMENTSPHEREAUX_H
#define RADIALDISPLACEMENTSPHEREAUX_H

#include "AuxKernel.h"

class RadialDisplacementSphereAux;

template <>
InputParameters validParams<RadialDisplacementSphereAux>();

/**
 * Calculates the radial displacement for spherical geometries.
 * Works for 3D, 2D axisymmetric, and 1D geometries
 */

class RadialDisplacementSphereAux : public AuxKernel
{
public:
  RadialDisplacementSphereAux(const InputParameters & parameters);

  virtual ~RadialDisplacementSphereAux() {}

protected:
  /// Compute the value of the radial displacement
  virtual Real computeValue();

  /// Type of coordinate system
  Moose::CoordinateSystemType _coord_system;

  /// Number of displacment components.
  unsigned int _ndisp;
  /// Coupled variable values of the displacement components.
  std::vector<const VariableValue *> _disp_vals;

  /// Point used to define an origin for 2D axisymmetric or
  /// 3D Cartesian systems.
  RealVectorValue _origin;
};

#endif // RADIALDISPLACEMENTSPHEREAUX_H
