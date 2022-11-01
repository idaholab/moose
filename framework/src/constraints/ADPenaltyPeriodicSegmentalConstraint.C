//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPenaltyPeriodicSegmentalConstraint.h"

namespace
{
const InputParameters &
setADPenaltyPeriodicSegParam(const InputParameters & params_in)
{
  // Reset the scalar_variable parameter to a relevant name for this physics
  InputParameters & ret = const_cast<InputParameters &>(params_in);
  ret.set<VariableName>("scalar_variable") = {params_in.get<VariableName>("kappa")};
  return ret;
}
}

registerMooseObject("MooseApp", ADPenaltyPeriodicSegmentalConstraint);

InputParameters
ADPenaltyPeriodicSegmentalConstraint::validParams()
{
  InputParameters params = ADMortarScalarBase::validParams();
  params.addClassDescription(
      "ADPenaltyPeriodicSegmentalConstraint enforces macro-micro periodic conditions between "
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

ADPenaltyPeriodicSegmentalConstraint::ADPenaltyPeriodicSegmentalConstraint(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ADMortarScalarBase>(setADPenaltyPeriodicSegParam(parameters)),
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
ADPenaltyPeriodicSegmentalConstraint::precalculateResidual()
{
  precalculateStability();
}
// Compute the temperature jump for current quadrature point
void
ADPenaltyPeriodicSegmentalConstraint::initScalarQpResidual()
{
  precalculateMaterial();
}

ADReal
ADPenaltyPeriodicSegmentalConstraint::computeQpResidual(const Moose::MortarType mortar_type)
{

  /// Compute penalty parameter times x-jump times average heat flux

  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  ADRealVectorValue kappa_vec(_kappa[0], _kappa[1], 0);
  Moose::derivInsert(kappa_vec(0).derivatives(), _kappa_var_ptr->dofIndices()[0], 1);
  Moose::derivInsert(kappa_vec(1).derivatives(), _kappa_var_ptr->dofIndices()[1], 1);
  ADReal r = _tau_s * (kappa_vec * dx);

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

ADReal
ADPenaltyPeriodicSegmentalConstraint::computeScalarQpResidual()
{

  /// Stability/penalty term for residual of scalar variable
  ADReal r = _tau_s * _temp_jump_global;
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);

  r *= -dx(_h);

  ADRealVectorValue kappa_vec(_kappa[0], _kappa[1], 0);
  Moose::derivInsert(kappa_vec(0).derivatives(), _kappa_var_ptr->dofIndices()[0], 1);
  Moose::derivInsert(kappa_vec(1).derivatives(), _kappa_var_ptr->dofIndices()[1], 1);
  RealVectorValue kappa_aux_vec(_kappa_aux[0], _kappa_aux[1], 0);

  r += dx(_h) * _tau_s * (kappa_vec * dx);
  r -= dx(_h) * (kappa_aux_vec * _normals[_qp]);

  return r;
}

void
ADPenaltyPeriodicSegmentalConstraint::precalculateStability()
{
  // Example showing how the penalty could be loaded from some function

  _tau_s = _pen_scale;
}

// Compute temperature jump and flux average/jump
void
ADPenaltyPeriodicSegmentalConstraint::precalculateMaterial()
{
  _temp_jump_global = (_u_primary[_qp] - _u_secondary[_qp]);
}
