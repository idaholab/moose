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
 * Auxiliary kernel responsible for computing the components of the flux vector
 * in diffusion problems using the FV formulation
 */
class DiffusionFluxFVAux : public AuxKernel
{
public:
  static InputParameters validParams();

  DiffusionFluxFVAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Whether the normal component has been selected
  const bool _use_normal;

  /// Will hold 0, 1, or 2 corresponding to x, y, or z.
  const int _component;

  /// Holds the solution functor for which to compute the gradient
  const MooseVariableFVReal * const _diffusion_var;

  /// Holds the diffusivity from the material system if non-AD
  const MaterialProperty<Real> * const _diffusion_coef;
  /// Holds the diffusivity from the material system if AD
  const ADMaterialProperty<Real> * const _ad_diffusion_coef;

  /// normals at quadrature points at the interface
  const MooseArray<Point> & _normals;
};
