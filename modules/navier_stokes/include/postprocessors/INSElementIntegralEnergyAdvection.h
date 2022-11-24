//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"

/**
 * Template class for computing the global energy loss/gain due to advection
 */
template <bool is_ad>
class INSElementIntegralEnergyAdvectionTempl : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  INSElementIntegralEnergyAdvectionTempl(const InputParameters & parameters);

protected:
  Real computeQpIntegral() override;

  /// The constant-pressure specific heat capacity
  const GenericMaterialProperty<Real, is_ad> & _cp;
  /// The density
  const GenericMaterialProperty<Real, is_ad> & _rho;
  /// The gradient of temperature
  const VariableGradient & _grad_T;
  /// The velocity
  const VectorVariableValue & _velocity;
};

typedef INSElementIntegralEnergyAdvectionTempl<false> INSElementIntegralEnergyAdvection;
typedef INSElementIntegralEnergyAdvectionTempl<true> INSADElementIntegralEnergyAdvection;
