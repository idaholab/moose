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
 * Implements a linear or quadratic friction term for the momentum equation.
 */
class INSFVMomentumFriction : public FVElementalKernel
{
public:
  static InputParameters validParams();

  INSFVMomentumFriction(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

protected:
  /// The linear friction factor, for laminar flow
  const Moose::Functor<ADReal> * const _linear_friction;
  /// The quadratic friction factor, for turbulent flow
  const Moose::Functor<ADReal> * const _quadratic_friction;
  /// Boolean to select the right model
  const bool _use_linear_friction;
  /// drag quantity
  const Moose::Functor<ADReal> & _drag_quantity;
};
