//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * This is the material class used to compute the viscosity of the kEpsilon model
 */
class INSFVkEpsilonViscosityFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  INSFVkEpsilonViscosityFunctorMaterial(const InputParameters & parameters);

protected:
  /// The density
  const Moose::Functor<ADReal> & _rho;
  /// The turbulent kinetic energy
  const Moose::Functor<ADReal> & _k;
  /// The turbulent kinetic energy dissipation rate
  const Moose::Functor<ADReal> & _epsilon;
  /// The C_mu
  const Moose::Functor<ADReal> & _C_mu;
  /// Whether to preserve the sparsity pattern between iterations (needed for Newton solvers)
  const bool _preserve_sparsity_pattern;
};
