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
class NSFVMomentumFriction : public FVElementalKernel
{
public:
  static InputParameters validParams();

  NSFVMomentumFriction(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

protected:
  /// The linear friction factor, for laminar flow, as a material property
  const ADMaterialProperty<Real> * const _linear_friction_matprop;
  /// The quadratic friction factor, for turbulent flow, as a material property
  const ADMaterialProperty<Real> * const _quadratic_friction_matprop;
  /// Boolean to select the right model
  const bool _use_linear_friction_matprop;
  /// drag quantity
  const MooseArray<ADReal> & _drag_quantity;
};
