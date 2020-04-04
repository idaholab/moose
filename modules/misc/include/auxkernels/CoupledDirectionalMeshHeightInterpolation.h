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
 * Couples to some other value and modulates it by the mesh height in a direction.
 *
 * What this means is that a constant field will become a linear field that is zero
 * at the end of the domain that is most negative in the given direction and is that
 * given constant value at the extremum of the mesh in the given direction.
 *
 * This can be useful for specifying linear mesh "stretches".  If you are wanting to stretch
 * a mesh by "5"... put 5 into the field that this couples to and this AuxKernel can create
 * a linear "displacement" field that will give that stretch.
 */
class CoupledDirectionalMeshHeightInterpolation : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  static InputParameters validParams();

  CoupledDirectionalMeshHeightInterpolation(const InputParameters & parameters);

  virtual ~CoupledDirectionalMeshHeightInterpolation() {}

protected:
  virtual Real computeValue();

  /// The value of a coupled variable to modulate
  const VariableValue & _coupled_val;

  /// The direction to interpolate in
  unsigned int _direction;

  Real _direction_min;
  Real _direction_max;
};
