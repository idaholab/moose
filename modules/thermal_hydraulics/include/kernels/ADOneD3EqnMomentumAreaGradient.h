//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 * Computes the area gradient term in the momentum equation.
 *
 * This Kernel computes the following area gradient term in momentum equation:
 * \f[
 *   p_k \frac{\partial A}{\partial x} .
 * \f]
 */
class ADOneD3EqnMomentumAreaGradient : public ADKernel
{
public:
  ADOneD3EqnMomentumAreaGradient(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  const VariableGradient & _area_grad;

  /// The direction of the flow channel
  const MaterialProperty<RealVectorValue> & _dir;

  const ADMaterialProperty<Real> & _pressure;

public:
  static InputParameters validParams();
};
