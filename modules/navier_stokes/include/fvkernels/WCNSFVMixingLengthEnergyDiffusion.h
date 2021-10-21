//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"
#include "INSFVVelocityVariable.h"

/**
 * Computes the turbulent diffusion of energy term in the weakly compressible formulation
 * of the energy equation, using functor material properties
 */
class WCNSFVMixingLengthEnergyDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();

  WCNSFVMixingLengthEnergyDiffusion(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const INSFVVelocityVariable * const _u_var;
  /// y-velocity
  const INSFVVelocityVariable * const _v_var;
  /// z-velocity
  const INSFVVelocityVariable * const _w_var;

  /// the fluid density
  const Moose::Functor<ADReal> & _rho;

  /// the specific heat capacity
  const Moose::Functor<ADReal> & _cp;

  /// Turbulent eddy mixing length
  const MooseVariableFVReal & _mixing_len;

  /// Turbulent Schmidt number (or turbulent Prandtl number)
  const Real & _schmidt_number;
};
