

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

#include "INSFVVelocityVariable.h"
/**
 * Computes the turbulent conductivity.
 * Implements two near-wall treatements: equilibrium and non-equilibrium wall functions.
 */
class EffectiveTurbulentThermalConductivityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  EffectiveTurbulentThermalConductivityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Specific heat at Constant Pressure
  const Moose::Functor<Real> & _cp;
  /// Turbulent viscosity
  const Moose::Functor<Real> & _mu_t;
  /// Thermal Conductivity
  const Moose::Functor<Real> & _k;
  /// Turbulent Prandtl number
  const Moose::Functor<Real> & _Pr_t;

  /// Boolean to pick between k_eff or k_t
  const bool _turbulent_thermal_conductivity;
};
