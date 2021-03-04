//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PNSFVUpwindAdvectorAndAdvected.h"

class PNSFVMomentumOneOverGammaUpwindAdvectorAndAdvected : public PNSFVUpwindAdvectorAndAdvected
{
public:
  static InputParameters validParams();
  PNSFVMomentumOneOverGammaUpwindAdvectorAndAdvected(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// The interpolation method to use for one / porosity
  Moose::FV::InterpMethod _one_over_porosity_interp_method;
};
