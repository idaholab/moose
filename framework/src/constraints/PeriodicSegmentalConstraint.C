//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeriodicSegmentalConstraint.h"

registerMooseObject("MooseApp", PeriodicSegmentalConstraint);

InputParameters
PeriodicSegmentalConstraint::validParams()
{
  InputParameters params = MortarScalarBase::validParams();
  params.addClassDescription(
      "PeriodicSegmentalConstraint enforces macro-micro periodic conditions between "
      "secondary and primary sides of a mortar interface using Lagrange multipliers."
      "Must be used alongside EqualValueConstraint.");
  params.renameCoupledVar("scalar_variable", "epsilon", "Primary coupled scalar variable");
  params.addRequiredCoupledVar("sigma", "Controlled scalar averaging variable");

  return params;
}

PeriodicSegmentalConstraint::PeriodicSegmentalConstraint(const InputParameters & parameters)
  : DerivativeMaterialInterface<MortarScalarBase>(parameters),
    _kappa_aux_ptr(getScalarVar("sigma", 0)),
    _ka_order(_kappa_aux_ptr->order()),
    _kappa_aux(coupledScalarValue("sigma"))
{
  if (_kappa_aux_ptr->kind() != Moose::VarKindType::VAR_AUXILIARY)
    paramError("sigma",
               "Must assign auxiliary scalar variable to sigma, rather than nonlinear variable");
}

Real
PeriodicSegmentalConstraint::computeQpResidual(const Moose::MortarType mortar_type)
{
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  RealVectorValue kappa_vec(_kappa[0], 0, 0);
  if (_k_order == 2)
    kappa_vec(1) = _kappa[1];
  else if (_k_order == 3)
  {
    kappa_vec(1) = _kappa[1];
    kappa_vec(2) = _kappa[2];
  }
  Real r = -(kappa_vec * dx);

  switch (mortar_type)
  {
    case Moose::MortarType::Lower:
      r *= _test[_i][_qp];
      break;
    default:
      return 0;
  }
  return r;
}

Real
PeriodicSegmentalConstraint::computeScalarQpResidual()
{
  // Stability/penalty term for residual of scalar variable
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  Real r = -dx(_h) * _lambda[_qp];

  RealVectorValue kappa_aux_vec(_kappa_aux[0], 0, 0);
  if (_k_order == 2)
  {
    kappa_aux_vec(1) = _kappa_aux[1];
  }
  else if (_k_order == 3)
  {
    kappa_aux_vec(1) = _kappa_aux[1];
    kappa_aux_vec(2) = _kappa_aux[2];
  }

  r -= dx(_h) * (kappa_aux_vec * _normals[_qp]);

  return r;
}

Real
PeriodicSegmentalConstraint::computeQpOffDiagJacobianScalar(const Moose::MortarType mortar_type,
                                                            const unsigned int svar_num)
{
  if (svar_num != _kappa_var)
    return 0;

  // Stability/penalty term for Jacobian
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  Real jac = -dx(_h);

  switch (mortar_type)
  {
    case Moose::MortarType::Lower: // Residual_sign -1  ddeltaU_ddisp sign 1;
      jac *= _test[_i][_qp];
      break;
    default:
      return 0;
  }
  return jac;
}

Real
PeriodicSegmentalConstraint::computeScalarQpOffDiagJacobian(const Moose::MortarType mortar_type,
                                                            const unsigned int jvar_num)
{
  // Test if jvar is the ID of the primary variables and not some other random variable
  switch (mortar_type)
  {
    case Moose::MortarType::Lower:
      if (!_var || _var->number() != jvar_num)
        return 0;
      break;
    default:
      return 0;
  }

  // Stability/penalty term for Jacobian
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  Real jac = -dx(_h);

  switch (mortar_type)
  {
    case Moose::MortarType::Lower:
      jac *= (*_phi)[_j][_qp];
      break;
    default:
      return 0;
  }
  return jac;
}
