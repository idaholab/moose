//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * SwitchingFunctionConstraintEta is a constraint kernel that acts on the
 * lambda lagrange multiplier non-linear variables to
 * enforce \f$ \sum_n h_i(\eta_i) \equiv 1 \f$.
 */
class SwitchingFunctionConstraintEta
  : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  SwitchingFunctionConstraintEta(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  /// Switching function name
  VariableName _eta_name;

  ///@{ Switching function drivatives
  const MaterialProperty<Real> & _dh;
  const MaterialProperty<Real> & _d2h;
  std::vector<const MaterialProperty<Real> *> _d2ha;
  const JvarMap & _d2ha_map;
  ///@}

  /// Lagrange multiplier
  const VariableValue & _lambda;
  unsigned int _lambda_var;
};
