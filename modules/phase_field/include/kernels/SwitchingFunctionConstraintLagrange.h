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
#include "NonlinearSystem.h"

/**
 * SwitchingFunctionConstraintLagrange is a constraint kernel that acts on the
 * lambda lagrange multiplier non-linear variables to
 * enforce \f$ \sum_n h_i(\eta_i) - \epsilon\lambda \equiv 1 \f$.
 */
template <bool is_ad>
class SwitchingFunctionConstraintLagrangeTempl
  : public DerivativeMaterialInterface<JvarMapKernelInterface<GenericKernel<is_ad>>>
{
public:
  static InputParameters validParams();

  SwitchingFunctionConstraintLagrangeTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  /// Switching function names
  std::vector<MaterialPropertyName> _h_names;

  /// number of switching functions
  unsigned int _num_h;

  /// Switching functions
  std::vector<const GenericMaterialProperty<Real, is_ad> *> _h;

  /// shift factor
  Real _epsilon;

  usingGenericKernelMembers;
};

class SwitchingFunctionConstraintLagrange : public SwitchingFunctionConstraintLagrangeTempl<false>
{
public:
  SwitchingFunctionConstraintLagrange(const InputParameters & parameters);

protected:
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int) override;

  /// Switching function derivatives
  std::vector<std::vector<const MaterialProperty<Real> *>> _dh;

  /// map for getting the "etas" index from jvar
  const JvarMap & _eta_map;
};

using ADSwitchingFunctionConstraintLagrange = SwitchingFunctionConstraintLagrangeTempl<true>;