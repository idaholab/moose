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

  const Moose::Functor<ADRealVectorValue> & _vel;

  /// The advected quantity
  const Moose::Functor<ADReal> & _adv_quant;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;
};
