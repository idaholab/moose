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
#include "MathFVUtils.h"
#include "INSFVMomentumResidualObject.h"
#include "INSFVVelocityVariable.h"

/**
 * Computes the source and sink terms for the v2f elliptic blending function
 */
class INSFVTFSourceSink : public FVElementalKernel
{
public:
  static InputParameters validParams();

  INSFVTFSourceSink(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

protected:
  /// The dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const Moose::Functor<ADReal> & _u_var;
  /// y-velocity
  const Moose::Functor<ADReal> * _v_var;
  /// z-velocity
  const Moose::Functor<ADReal> * _w_var;

  /// Turbulent kinetic energy
  const Moose::Functor<ADReal> & _k;

  /// Turbulent kinetic energy dissipation rate
  const Moose::Functor<ADReal> & _epsilon;

  /// Turbulent normal wall fluctuations
  const Moose::Functor<ADReal> & _v2;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// Turbulent dynamic viscosity
  const Moose::Functor<ADReal> & _mu_t;

  /// Value of the first epsilon closure coefficient
  const Real _C1;

  /// Value of the second epsilon closure coefficient
  const Real _C2;

  /// Model paramters
  const Real _n;
  const Real _C_eta;
};
