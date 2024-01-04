

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
#include "INSFVVelocityVariable.h"

/**
 * Computes the turbuent viscosity for the k-Epsilon model.
 * Implements two near-wall treatments: equilibrium and non-equilibrium wall functions.
 */
class v2fLengthScaleSquaredAux : public AuxKernel
{
public:
  static InputParameters validParams();

  v2fLengthScaleSquaredAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Turbulent kinetic energy
  const Moose::Functor<ADReal> & _k;
  /// Turbulent kinetic energy dissipation rate
  const Moose::Functor<ADReal> & _epsilon;

  /// Density
  const Moose::Functor<ADReal> & _rho;
  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// Closure Coefficients
  const Real _C_L;
  const Real _C_eta;
};
