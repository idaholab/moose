//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVAdvectionKernel.h"

/**
 * An advection kernel that implements interpolation schemes specific to Navier-Stokes flow
 * physics
 */
class INSFVEnergyAdvection : public INSFVAdvectionKernel
{
public:
  static InputParameters validParams();
  INSFVEnergyAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;
  virtual bool hasMaterialTimeDerivative() const override { return true; }

  /// The advected heat quantity
  const Moose::Functor<ADReal> & _adv_quant;
};
