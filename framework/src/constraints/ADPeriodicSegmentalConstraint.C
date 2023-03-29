//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPeriodicSegmentalConstraint.h"

registerMooseObject("MooseApp", ADPeriodicSegmentalConstraint);

InputParameters
ADPeriodicSegmentalConstraint::validParams()
{
  InputParameters params = ADMortarScalarBase::validParams();
  params.addClassDescription(
      "ADPeriodicSegmentalConstraint enforces macro-micro periodic conditions between "
      "secondary and primary sides of a mortar interface using Lagrange multipliers."
      "Must be used alongside EqualValueConstraint.");
  params.renameCoupledVar("scalar_variable", "epsilon", "Primary coupled scalar variable");
  params.addRequiredCoupledVar("sigma", "Controlled scalar averaging variable");

  return params;
}

ADPeriodicSegmentalConstraint::ADPeriodicSegmentalConstraint(const InputParameters & parameters)
  : DerivativeMaterialInterface<ADMortarScalarBase>(parameters),
    _kappa_aux_ptr(getScalarVar("sigma", 0)),
    _ka_order(_kappa_aux_ptr->order()),
    _kappa_aux(coupledScalarValue("sigma"))
{
  if (_kappa_aux_ptr->kind() != Moose::VarKindType::VAR_AUXILIARY)
    paramError("sigma",
               "Must assign auxiliary scalar variable to sigma, rather than nonlinear variable");
}

ADReal
ADPeriodicSegmentalConstraint::computeQpResidual(const Moose::MortarType mortar_type)
{
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  ADRealVectorValue kappa_vec(_kappa[0], 0, 0);
  Moose::derivInsert(kappa_vec(0).derivatives(), _kappa_var_ptr->dofIndices()[0], 1);
  if (_k_order == 2)
  {
    kappa_vec(1) = _kappa[1];
    Moose::derivInsert(kappa_vec(1).derivatives(), _kappa_var_ptr->dofIndices()[1], 1);
  }
  else if (_k_order == 3)
  {
    kappa_vec(1) = _kappa[1];
    kappa_vec(2) = _kappa[2];
    Moose::derivInsert(kappa_vec(1).derivatives(), _kappa_var_ptr->dofIndices()[1], 1);
    Moose::derivInsert(kappa_vec(2).derivatives(), _kappa_var_ptr->dofIndices()[2], 1);
  }
  ADReal r = -(kappa_vec * dx);

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

ADReal
ADPeriodicSegmentalConstraint::computeScalarQpResidual()
{
  // Stability/penalty term for residual of scalar variable
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  ADReal r = -dx(_h) * _lambda[_qp];

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
