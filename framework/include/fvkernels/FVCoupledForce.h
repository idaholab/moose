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
class FVCoupledForce : public FVElementalKernel
{
public:
  static InputParameters validParams();

  FVCoupledForce(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

protected:
  /// The coupled functor applying the force
  const Moose::Functor<ADReal> & _v;
  /// An optional coefficient multiplying the coupled force
  const Real _coef;
};
