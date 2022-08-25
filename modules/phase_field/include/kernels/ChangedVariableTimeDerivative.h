//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeDerivative.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * This calculates the time derivative for a variable multiplied by a generalized susceptibility
 */
class ChangedVariableTimeDerivative
  : public DerivativeMaterialInterface<JvarMapKernelInterface<TimeDerivative>>
{
public:
  static InputParameters validParams();

  ChangedVariableTimeDerivative(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Order parameter derivative
  const MaterialProperty<Real> & _dopdu;

  /// 2nd order parameter derivative
  const MaterialProperty<Real> & _d2opdu2;

  /// 2nd order parameter derivative w.r.t. coupled variables
  std::vector<const MaterialProperty<Real> *> _d2opdudarg;
};
