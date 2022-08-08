//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyEqualValueConstraint.h"

registerMooseObject("MooseApp", PenaltyEqualValueConstraint);
registerMooseObject("MooseApp", ADPenaltyEqualValueConstraint);

template <bool is_ad>
InputParameters
PenaltyEqualValueConstraintTempl<is_ad>::validParams()
{
  InputParameters params = MortarConstraintTempl<is_ad>::validParams();
  params.addClassDescription(
      "PenaltyEqualValueConstraint enforces solution continuity between secondary and "
      "primary sides of a mortar interface using a penalty approach (no Lagrange multipliers "
      "needed)");
  params.addRequiredRangeCheckedParam<Real>(
      "penalty_value",
      "penalty_value>0",
      "Penalty value used to impose a generalized force capturing the mortar constraint equation");
  return params;
}

template <bool is_ad>
PenaltyEqualValueConstraintTempl<is_ad>::PenaltyEqualValueConstraintTempl(
    const InputParameters & parameters)
  : MortarConstraintTempl<is_ad>(parameters),
    _penalty_value(this->template getParam<Real>("penalty_value"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PenaltyEqualValueConstraintTempl<is_ad>::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Secondary:
    {
      auto residual =
          -(_u_primary[_qp] - _u_secondary[_qp]) * _penalty_value * _test_secondary[_i][_qp];

      return residual;
    }

    case Moose::MortarType::Primary:
    {
      auto residual =
          (_u_primary[_qp] - _u_secondary[_qp]) * _penalty_value * _test_primary[_i][_qp];

      return residual;
    }

    default:
      return 0;
  }
}

template <>
ADReal
PenaltyEqualValueConstraintTempl<true>::computeQpJacobian(
    Moose::ConstraintJacobianType /*jacobian_type*/, unsigned int /*jvar*/)
{
  mooseError("ADPenaltyEqualValueConstraint does not implement manual Jacobian calculation.");
}

template <>
Real
PenaltyEqualValueConstraintTempl<false>::computeQpJacobian(
    Moose::ConstraintJacobianType jacobian_type, unsigned int jvar)
{
  typedef Moose::ConstraintJacobianType JType;

  switch (jacobian_type)
  {
    case JType::SecondarySecondary:
      if (jvar == _secondary_var.number())
        return (*_phi)[_j][_qp] * _penalty_value * _test_secondary[_i][_qp];
      break;

    case JType::SecondaryPrimary:
      if (jvar == _primary_var.number())
        return -(*_phi)[_j][_qp] * _penalty_value * _test_secondary[_i][_qp];
      break;

    case JType::PrimarySecondary:
      if (jvar == _secondary_var.number())
        return -(*_phi)[_j][_qp] * _penalty_value * _test_primary[_i][_qp];
      break;

    case JType::PrimaryPrimary:
      if (jvar == _primary_var.number())
        return (*_phi)[_j][_qp] * _penalty_value * _test_primary[_i][_qp];
      break;

    default:
      return 0;
  }

  return 0;
}

template class PenaltyEqualValueConstraintTempl<false>;
template class PenaltyEqualValueConstraintTempl<true>;
