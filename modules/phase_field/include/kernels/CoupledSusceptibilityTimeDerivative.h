//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CoupledTimeDerivative.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"
// Forward Declaration

/**
 * This calculates a modified coupled time derivative that multiplies the time derivative of a
 * coupled variable by a function of the variables
 */
class CoupledSusceptibilityTimeDerivative
  : public DerivativeMaterialInterface<JvarMapKernelInterface<CoupledTimeDerivative>>
{
public:
  static InputParameters validParams();

  CoupledSusceptibilityTimeDerivative(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// The function multiplied by the coupled time derivative
  const MaterialProperty<Real> & _F;

  /// function derivative w.r.t. the kernel variable
  const MaterialProperty<Real> & _dFdu;

  /// function derivatives w.r.t. coupled variables
  std::vector<const MaterialProperty<Real> *> _dFdarg;
};
