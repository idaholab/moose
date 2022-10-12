//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltyPeriodicSegmentalConstraint.h"

namespace
{
const InputParameters &
setPenaltyPeriodicSegParam(const InputParameters & params_in)
{
  // Reset the scalar_variable parameter to a relevant name for this physics
  InputParameters & ret = const_cast<InputParameters &>(params_in);
  ret.set<VariableName>("scalar_variable") = {params_in.get<VariableName>("kappa")};
  return ret;
}
}

registerMooseObject("MooseApp", PenaltyPeriodicSegmentalConstraint);

InputParameters
PenaltyPeriodicSegmentalConstraint::validParams()
{
  InputParameters params = MortarScalarBase::validParams();
  params.addClassDescription(
      "PenaltyPeriodicSegmentalConstraint enforces macro-micro periodic conditions between "
      "secondary and primary sides of a mortar interface using a penalty approach "
      "(no Lagrange multipliers needed). Must be used alongside PenaltyEqualValueConstraint.");
  params.addRequiredParam<VariableName>("kappa", "Primary coupled scalar variable");
  params.addRequiredCoupledVar("kappa_aux", "Controlled scalar averaging variable");
  params.addParam<Real>(
      "penalty_value",
      1.0,
      "Penalty value used to impose a generalized force capturing the mortar constraint equation");

  return params;
}

PenaltyPeriodicSegmentalConstraint::PenaltyPeriodicSegmentalConstraint(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<MortarScalarBase>(setPenaltyPeriodicSegParam(parameters)),
    _temp_jump_global(),
    _tau_s(),
    _kappa_aux_var(coupledScalar("kappa_aux")),
    _ka_order(getScalarVar("kappa_aux", 0)->order()),
    _kappa_aux(coupledScalarValue("kappa_aux")),
    _pen_scale(getParam<Real>("penalty_value"))
{
}

// Compute the stability parameters to use for all quadrature points
void
PenaltyPeriodicSegmentalConstraint::precalculateResidual()
{
  precalculateStability();
}
void
PenaltyPeriodicSegmentalConstraint::precalculateJacobian()
{
  precalculateStability();
}
// Compute the temperature jump for current quadrature point
void
PenaltyPeriodicSegmentalConstraint::initScalarQpResidual()
{
  precalculateMaterial();
}

Real
PenaltyPeriodicSegmentalConstraint::computeQpResidual(const Moose::MortarType mortar_type)
{

  /// Compute penalty parameter times x-jump times average heat flux

  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  RealVectorValue kappa_vec(_kappa[0], _kappa[1], 0);
  Real r = _tau_s * (kappa_vec * dx);

  switch (mortar_type)
  {
    case Moose::MortarType::Secondary:
      r *= _test_secondary[_i][_qp];
      break;
    case Moose::MortarType::Primary:
      r *= -_test_primary[_i][_qp];
      break;
    case Moose::MortarType::Lower:
      return 0;
    default:
      return 0;
  }
  return r;
}

Real
PenaltyPeriodicSegmentalConstraint::computeScalarQpResidual()
{

  /// Stability/penalty term for residual of scalar variable
  Real r = _tau_s * _temp_jump_global;
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);

  r *= -dx(_h);

  RealVectorValue kappa_vec(_kappa[0], _kappa[1], 0);
  RealVectorValue kappa_aux_vec(_kappa_aux[0], _kappa_aux[1], 0);

  r += dx(_h) * _tau_s * (kappa_vec * dx);
  r -= dx(_h) * (kappa_aux_vec * _normals[_qp]);

  return r;
}

Real
PenaltyPeriodicSegmentalConstraint::computeScalarQpJacobian()
{

  /// Stability/penalty term for Jacobian of scalar variable
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);

  Real jac = dx(_h) * _tau_s * dx(_l);

  return jac;
}

Real
PenaltyPeriodicSegmentalConstraint::computeQpOffDiagJacobianScalar(
    const Moose::MortarType mortar_type, const unsigned int svar_num)
{
  if (svar_num != _kappa_var)
    return 0;

  /// Stability/penalty term for Jacobian

  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  Real jac = _tau_s;

  switch (mortar_type)
  {

    case Moose::MortarType::Secondary: // Residual_sign -1  ddeltaU_ddisp sign 1;
      jac *= _test_secondary[_i][_qp] * dx(_h);
      break;
    case Moose::MortarType::Primary: // Residual_sign -1  ddeltaU_ddisp sign -1;
      jac *= -_test_primary[_i][_qp] * dx(_h);
      break;

    default:
      return 0;
  }
  return jac;
}

Real
PenaltyPeriodicSegmentalConstraint::computeScalarQpOffDiagJacobian(
    const Moose::MortarType mortar_type, const unsigned int jvar_num)
{
  // Test if jvar is the ID of the primary variables and not some other random variable
  switch (mortar_type)
  {
    case Moose::MortarType::Secondary:
      if (jvar_num != _secondary_var.number())
        return 0;
      break;
    case Moose::MortarType::Primary:
      if (jvar_num != _primary_var.number())
        return 0;
      break;
    default:
      return 0;
  }

  /// Stability/penalty term for Jacobian
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  Real jac = _tau_s;

  switch (mortar_type)
  {
    case Moose::MortarType::Secondary: // Residual_sign -1  ddeltaU_ddisp sign 1;
                                       // This assumes Galerkin, i.e. (*_phi)[_i][_qp] =
                                       // _test_secondary[_i][_qp]
      jac *= _test_secondary[_i][_qp] * dx(_h);
      break;
    case Moose::MortarType::Primary: // Residual_sign -1  ddeltaU_ddisp sign -1;
                                     // This assumes Galerkin, i.e. (*_phi)[_i][_qp] =
                                     // _test_primary[_i][_qp]
      jac *= -_test_primary[_i][_qp] * dx(_h);
      break;

    default:
      return 0;
  }
  return jac;
}

void
PenaltyPeriodicSegmentalConstraint::precalculateStability()
{
  // Example showing how the penalty could be loaded from some function

  _tau_s = _pen_scale;
}

// Compute temperature jump and flux average/jump
void
PenaltyPeriodicSegmentalConstraint::precalculateMaterial()
{
  _temp_jump_global = (_u_primary[_qp] - _u_secondary[_qp]);
}
