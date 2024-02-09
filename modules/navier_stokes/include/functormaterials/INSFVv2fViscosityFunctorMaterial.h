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
class INSFVv2fViscosityFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  INSFVv2fViscosityFunctorMaterial(const InputParameters & parameters);

protected:
  /// Turbulent kinetic energy
  const Moose::Functor<ADReal> & _k;
  /// Turbulent kinetic energy dissipation rate
  const Moose::Functor<ADReal> & _epsilon;
  /// Turbulent wall normal
  const Moose::Functor<ADReal> & _v2;

  /// Density
  const Moose::Functor<ADReal> & _rho;
  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// C-mu closure coefficient
  const Real _C_mu_2;
  const Real _C_mu;
};
