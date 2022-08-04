//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarConstraint.h"
#include "ADMortarConstraint.h"

// Forward declaration
template <bool is_ad>
using MortarConstraintTempl =
    typename std::conditional<is_ad, ADMortarConstraint, MortarConstraint>::type;

/**
 * Constrain the value of a variable to be the same on both sides of an
 * interface using a generalized force stemming from a penalty-based enforcement.
 */
template <bool is_ad>
class PenaltyEqualValueConstraintTempl : public MortarConstraintTempl<is_ad>
{
public:
  static InputParameters validParams();

  PenaltyEqualValueConstraintTempl(const InputParameters & parameters);

protected:
  GenericReal<is_ad> computeQpResidual(Moose::MortarType mortar_type) final;
  GenericReal<is_ad> computeQpJacobian(Moose::ConstraintJacobianType jacobian_type,
                                       unsigned int jvar);

  /// Penalty value used to enforce the constraint
  const Real _penalty_value;

  using MortarConstraintTempl<is_ad>::_u_primary;
  using MortarConstraintTempl<is_ad>::_u_secondary;
  using MortarConstraintTempl<is_ad>::_test_primary;
  using MortarConstraintTempl<is_ad>::_test_secondary;
  using MortarConstraintTempl<is_ad>::_qp;
  using MortarConstraintTempl<is_ad>::_i;
  using MortarConstraintTempl<is_ad>::_j;
};

typedef PenaltyEqualValueConstraintTempl<false> PenaltyEqualValueConstraint;
typedef PenaltyEqualValueConstraintTempl<true> ADPenaltyEqualValueConstraint;
