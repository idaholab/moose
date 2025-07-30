//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Auxiliary kernel responsible for computing the components of the flux vector
 * in diffusion problems using the FV formulation
 */
class FVFunctorGradientAux : public AuxKernel
{
public:
  static InputParameters validParams();

  FVFunctorGradientAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Whether the normal component has been selected
  const bool _use_normal;

  /// Will hold 0, 1, or 2 corresponding to x, y, or z.
  const int _component;

  /// Holds the solution functor for which to compute the gradient
  const MooseVariableFVReal * const _gradient_var;

  /// scaling factor
  const Real _scaling_factor;

  /// normals at quadrature points at the interface
  const MooseArray<Point> & _normals;
};
