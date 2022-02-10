//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ODETimeDerivative.h"

/**
 * Time derivative multiplied by a coefficient for ODEs
 */
class ODECoefTimeDerivative : public ODETimeDerivative
{
public:
  ODECoefTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Coefficient that the time derivative terms is multiplied with
  const Real & _coef;

public:
  static InputParameters validParams();
};
