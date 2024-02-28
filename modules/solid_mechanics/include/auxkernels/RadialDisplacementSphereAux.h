//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Calculates the radial displacement for spherical geometries.
 * Works for 3D, 2D axisymmetric, and 1D geometries
 */

class RadialDisplacementSphereAux : public AuxKernel
{
public:
  static InputParameters validParams();

  RadialDisplacementSphereAux(const InputParameters & parameters);

  virtual ~RadialDisplacementSphereAux() {}

protected:
  /// Compute the value of the radial displacement
  virtual Real computeValue();

  /// Type of coordinate system
  Moose::CoordinateSystemType _coord_system;

  /// Number of displacment components.
  const unsigned int _ndisp;
  /// Coupled variable values of the displacement components.
  const std::vector<const VariableValue *> _disp_vals;

  /// Point used to define an origin for 2D axisymmetric or
  /// 3D Cartesian systems.
  RealVectorValue _origin;
};
