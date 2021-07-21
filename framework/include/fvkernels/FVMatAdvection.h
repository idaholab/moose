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

class FVMatAdvection : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVMatAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const FunctorMaterialProperty<ADRealVectorValue> & _vel;

  /// The advected quantity on the elem
  const FunctorInterface<ADReal> & _adv_quant;

  /// Central differencing to compute the velocity
  std::unique_ptr<Moose::FV::Limiter> _cd_limiter;

  /// The limiting method to use for the advected quantity
  std::unique_ptr<Moose::FV::Limiter> _limiter;

  /// The interfacial velocity. We cache this in the residual computation in case a derived class
  /// might want to use it
  ADRealVectorValue _v;
};
