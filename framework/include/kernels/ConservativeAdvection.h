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

/**
 * Advection of the variable by the velocity provided by the user.
 * Options for numerical stabilization are: none; full upwinding
 */
template <bool is_ad>
class ConservativeAdvectionTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  ConservativeAdvectionTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;

  /// advection velocity
  const GenericVectorVariableValue<is_ad> & _velocity;

  /// advected quantity
  const MooseArray<GenericReal<is_ad>> & _adv_quant;

  /// enum to make the code clearer
  enum class JacRes
  {
    CALCULATE_RESIDUAL = 0,
    CALCULATE_JACOBIAN = 1
  };

  /// Type of upwinding
  const enum class UpwindingType { none, full } _upwinding;

  /// Nodal value of u, used for full upwinding
  const VariableValue & _u_nodal;

  /// In the full-upwind scheme, whether a node is an upwind node
  std::vector<bool> _upwind_node;

  /// In the full-upwind scheme d(total_mass_out)/d(variable_at_node_i)
  std::vector<Real> _dtotal_mass_out;

  /// Returns - _grad_test * velocity
  GenericReal<is_ad> negSpeedQp() const;

  /// Calculates the fully-upwind Residual and Jacobian (depending on res_or_jac)
  void fullUpwind(JacRes res_or_jac);

  usingGenericKernelMembers;
};

typedef ConservativeAdvectionTempl<false> ConservativeAdvection;
typedef ConservativeAdvectionTempl<true> ADConservativeAdvection;
