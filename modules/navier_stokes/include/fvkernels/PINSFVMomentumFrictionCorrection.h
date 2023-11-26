//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVFluxKernel.h"

class PINSFVMomentumFrictionCorrection : public INSFVFluxKernel
{
public:
  static InputParameters validParams();
  PINSFVMomentumFrictionCorrection(const InputParameters & params);
  using INSFVFluxKernel::gatherRCData;
  void gatherRCData(const FaceInfo & fi) override final;

protected:
  /// Darcy coefficient
  const Moose::Functor<ADRealVectorValue> * const _D;
  /// Forchheimer coefficient
  const Moose::Functor<ADRealVectorValue> * const _F;

  /// Booleans to select the right models
  const bool _use_Darcy_friction_model;
  const bool _use_Forchheimer_friction_model;

  /// Density as a functor
  const Moose::Functor<ADReal> & _rho;
  /// Dynamic viscosity as a functor
  const Moose::Functor<ADReal> & _mu;
  /// Speed (norm of the interstitial velocity) as a functor
  const Moose::Functor<ADReal> * const _speed;

  /// Parameter for scaling the consistent pressure interpolation
  Real _consistent_scaling;
};
