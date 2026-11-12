//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * SwitchingFunctionConstraintEta is a constraint kernel that acts on the
 * lambda lagrange multiplier non-linear variables to
 * enforce \f$ \sum_n h_i(\eta_i) \equiv 1 \f$.
 */
template <bool is_ad>
class SwitchingFunctionConstraintEtaTempl
  : public DerivativeMaterialInterface<JvarMapKernelInterface<GenericKernel<is_ad>>>
{
public:
  static InputParameters validParams();

  SwitchingFunctionConstraintEtaTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  /// Switching function name
  VariableName _eta_name;

  ///@{ Switching function drivatives
  const GenericMaterialProperty<Real, is_ad> & _dh;
  ///@}

  /// Lagrange multiplier
  const GenericVariableValue<is_ad> & _lambda;

  usingGenericKernelMembers;
};

class SwitchingFunctionConstraintEta : public SwitchingFunctionConstraintEtaTempl<false>
{
public:
  SwitchingFunctionConstraintEta(const InputParameters & parameters);

protected:
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int) override;

  const MaterialProperty<Real> & _d2h;
  std::vector<const MaterialProperty<Real> *> _d2ha;
  const JvarMap & _d2ha_map;

  unsigned int _lambda_var;
};

using ADSwitchingFunctionConstraintEta = SwitchingFunctionConstraintEtaTempl<true>;
