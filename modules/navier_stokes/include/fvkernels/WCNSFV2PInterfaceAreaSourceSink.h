//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"
#include "INSFVVelocityVariable.h"

/**
 * Computes source the sink terms for the interface area in the mixture model of two-phase flows.
 */
class WCNSFV2PInterfaceAreaSourceSink : public FVElementalKernel
{
public:
  static InputParameters validParams();

  WCNSFV2PInterfaceAreaSourceSink(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

protected:
  /// The dimension of the domain
  const unsigned int _dim;

  /// x-velocity
  const Moose::Functor<ADReal> & _u_var;
  /// y-velocity
  const Moose::Functor<ADReal> * _v_var;
  /// z-velocity
  const Moose::Functor<ADReal> * _w_var;

  /// Characterisitc Length
  const Moose::Functor<ADReal> & _characheristic_length;

  /// Mixture Density
  const Moose::Functor<ADReal> & _rho_mixture;

  /// Dispersed Phase Density
  const Moose::Functor<ADReal> & _rho_d;

  /// Pressure field
  const Moose::Functor<ADReal> & _pressure;

  /// Interface Mass Exchange Coeefficient
  const Moose::Functor<ADReal> & _mass_exchange_coefficient;

  /// Void Fraction
  const Moose::Functor<ADReal> & _f_d;

  /// Maximum Void Fraction
  const Real _f_d_max;

  /// Surface Tension
  const Moose::Functor<ADReal> & _sigma;

  /// Particle Diameter
  const Moose::Functor<ADReal> & _particle_diameter;

  /// Cutoff fraction at which the full mass transfer model is activated
  const Real _cutoff_fraction;

  /// Internal closure coefficients
  static constexpr Real _gamma_c = 0.188;
  static constexpr Real _Kc = 0.129;
  static constexpr Real _gamma_b = 0.264;
  static constexpr Real _Kb = 1.37;
  static constexpr Real _shape_factor = 6.0;
};
