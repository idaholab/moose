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
 * Computes the average "flux" of a coupled variable and puts it in an elemental field.
 *
 * Only works on boundaries.
 */
class FluxAverageAux : public AuxKernel
{
public:
  static InputParameters validParams();

  FluxAverageAux(const InputParameters & parameters);

  virtual ~FluxAverageAux() {}

protected:
  virtual Real computeValue();

  /// The "material" property
  Real _diffusivity;

  /// Coupled gradient
  const VariableGradient & _coupled_gradient;

  /// The variable we're coupled to
  MooseVariable & _coupled_var;

  /// normals at quadrature points
  const MooseArray<Point> & _normals;
};
