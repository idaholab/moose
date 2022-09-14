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

/**
 * Simple class to demonstrate off diagonal Jacobian contributions.
 */
class PINSFVTKEDSourceSink : public FVElementalKernel
{
public:
  static InputParameters validParams();

  PINSFVTKEDSourceSink(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

protected:
  /// The dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const INSFVVelocityVariable * const _u_var;
  /// y-velocity
  const INSFVVelocityVariable * const _v_var;
  /// z-velocity
  const INSFVVelocityVariable * const _w_var;

  /// Turbulent kinetic energy
  //const INSFVVariable * const _k;
  const Moose::Functor<ADReal> & _k;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Turbulent dynamic viscosity
  const Moose::Functor<ADReal> & _mu_t;

  /// the porosity
  const Moose::Functor<ADReal> & _porosity;

  /// whether the diffusivity should be multiplied by porosity
  const bool _porosity_factored_in;

  /// functor for the first turbulent coefficient
  const Moose::Functor<ADReal> & _C1_eps;

  /// functor for the first turbulent coefficient
  const Moose::Functor<ADReal> & _C2_eps;

  /// Maximum mixing length allowed for the domain
  const Real _max_mixing_length;

  /// Linearized model?
  const bool _linearized_model;

  /// Linearization coupled functor
  const Moose::Functor<ADReal> & _linear_variable;

  /// Apply realizable constraints?
  const bool _realizable_constraint;
};