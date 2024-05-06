//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVElementalKernel.h"

/**
 * Imposes a friction force on the momentum equation in porous media in Rhie-Chow contexts
 */
class PINSFVMomentumFriction : public INSFVElementalKernel
{
public:
  static InputParameters validParams();
  PINSFVMomentumFriction(const InputParameters & params);

  using INSFVElementalKernel::gatherRCData;
  void gatherRCData(const Elem &) override;

protected:
  ADReal computeSegregatedContribution() override;

  /// Computes the friction coefficient which gets multiplied by the velocity
  ADReal computeFrictionWCoefficient(const Moose::ElemArg & elem_arg,
                                     const Moose::StateArg & state);

  /// Darcy coefficient
  const Moose::Functor<ADRealVectorValue> * const _D;
  /// Forchheimer coefficient
  const Moose::Functor<ADRealVectorValue> * const _F;
  /// Booleans to select the right models
  const bool _use_Darcy_friction_model;
  const bool _use_Forchheimer_friction_model;
  const bool _is_porous_medium;
  const bool _standard_friction_formulation;

  /// Dynamic viscosity
  const Moose::Functor<ADReal> * const _mu;
  /// Density as a functor
  const Moose::Functor<ADReal> & _rho;
  /// Speed (norm of the interstitial velocity) as a functor
  const Moose::Functor<ADReal> * const _speed;

  /// The dimension of the domain
  const unsigned int _dim;
  /// x-velocity
  const Moose::Functor<ADReal> * _u_var;
  /// y-velocity
  const Moose::Functor<ADReal> * _v_var;
  /// z-velocity
  const Moose::Functor<ADReal> * _w_var;
  /// Adding porosity functor
  const Moose::Functor<ADReal> & _epsilon;
};
