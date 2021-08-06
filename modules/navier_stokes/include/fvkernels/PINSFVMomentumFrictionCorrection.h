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
  const Moose::Functor<ADRealVectorValue> * const _cL;
  /// Forchheimer coefficient
  const Moose::Functor<ADRealVectorValue> * const _cQ;

  /// Booleans to select the right models
  const bool _use_Darcy_friction_model;
  const bool _use_Forchheimer_friction_model;

  /// Porosity to compute the interstitial velocity from the superficial velocity
  const Moose::Functor<ADReal> & _eps;
  /// Density as a functor
  const Moose::Functor<ADReal> & _rho;

  /// Parameter for scaling the consistent pressure interpolation
  Real _consistent_scaling;
};
