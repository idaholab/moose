//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADTestPeriodicSole.h"

registerMooseObject("MooseTestApp", ADTestPeriodicSole);

InputParameters
ADTestPeriodicSole::validParams()
{
  InputParameters params = ADMortarScalarBase::validParams();
  params.renameCoupledVar("scalar_variable", "kappa", "Primary coupled scalar variable");
  params.addRequiredCoupledVar("kappa_aux", "Controlled scalar averaging variable");
  params.addRequiredCoupledVar("kappa_other", "Other component of coupled scalar variable");
  params.addRequiredParam<unsigned int>("component", "Which direction this kernel acts in");
  params.addParam<Real>("pen_scale", 1.0, "Increase or decrease the penalty");

  return params;
}

ADTestPeriodicSole::ADTestPeriodicSole(const InputParameters & parameters)
  : DerivativeMaterialInterface<ADMortarScalarBase>(parameters),
    _temp_jump_global(),
    _tau_s(),
    _kappa_aux_var(coupledScalar("kappa_aux")),
    _ka_order(getScalarVar("kappa_aux", 0)->order()),
    _kappa_aux(coupledScalarValue("kappa_aux")),
    _alpha(getParam<unsigned int>("component")),
    _kappao_var_ptr(getScalarVar("kappa_other", 0)),
    _kappao_var(coupledScalar("kappa_other")),
    _ko_order(getScalarVar("kappa_other", 0)->order()),
    _kappa_other(adCoupledScalarValue("kappa_other")),
    _current_elem_volume(_assembly.elemVolume()),
    _current_side_volume(_assembly.sideElemVolume()),
    _pen_scale(getParam<Real>("pen_scale"))
{
}

// Compute the stability parameters to use for all quadrature points
void
ADTestPeriodicSole::precalculateResidual()
{
  precalculateStability();
}
void
// Compute the temperature jump for current quadrature point
ADTestPeriodicSole::initScalarQpResidual()
{
  precalculateMaterial();
}

ADReal
ADTestPeriodicSole::computeQpResidual(const Moose::MortarType mortar_type)
{

  /// Compute penalty parameter times x-jump times average heat flux

  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  // ONLY the component for this constraint will contribute here;
  // other one is handled in the other constraint
  ADRealVectorValue kappa_vec(0, 0, 0);
  if (_alpha == 0)
  {
    // ADRealVectorValue kappa_vec(_kappa[0], 0, 0);
    kappa_vec(0) = _kappa[0];
    Moose::derivInsert(kappa_vec(0).derivatives(), _kappa_var_ptr->dofIndices()[0], 1);
  }
  else
  {
    kappa_vec(1) = _kappa[0];
    // ADRealVectorValue kappa_vec(0, _kappa[0], 0);
    Moose::derivInsert(kappa_vec(1).derivatives(), _kappa_var_ptr->dofIndices()[0], 1);
  }
  ADReal r = (_pen_scale * _tau_s) * (kappa_vec * dx);

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
ADTestPeriodicSole::computeScalarQpResidual()
{

  /// Stability/penalty term for residual of scalar variable
  ADReal r = (_pen_scale * _tau_s) * _temp_jump_global;
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);

  r *= -dx(_alpha);

  ADRealVectorValue kappa_vec(0, 0, 0);
  if (_alpha == 0)
  {
    // ADRealVectorValue kappa_vec(_kappa[0], _kappa_other[0], 0);
    kappa_vec(0) = _kappa[0];
    kappa_vec(1) = _kappa_other[0];
    Moose::derivInsert(kappa_vec(0).derivatives(), _kappa_var_ptr->dofIndices()[0], 1);
    Moose::derivInsert(kappa_vec(1).derivatives(), _kappao_var_ptr->dofIndices()[0], 1);
  }
  else
  {
    // ADRealVectorValue kappa_vec(_kappa_other[0], _kappa[0], 0);
    kappa_vec(0) = _kappa_other[0];
    kappa_vec(1) = _kappa[0];
    Moose::derivInsert(kappa_vec(0).derivatives(), _kappao_var_ptr->dofIndices()[0], 1);
    Moose::derivInsert(kappa_vec(1).derivatives(), _kappa_var_ptr->dofIndices()[0], 1);
  }
  RealVectorValue kappa_aux_vec(_kappa_aux[0], _kappa_aux[1], 0);

  r += dx(_alpha) * (_pen_scale * _tau_s) * (kappa_vec * dx);
  r -= dx(_alpha) * (kappa_aux_vec * _normals[_qp]);

  return r;
}

void
ADTestPeriodicSole::precalculateStability()
{
  // const unsigned int elem_b_order = _secondary_var.order();
  // double h_elem =
  //     _current_elem_volume / _current_side_volume * 1. / Utility::pow<2>(elem_b_order);
  // h_elem = 10.0;
  const double h_elem = 1.0;

  _tau_s = (pencoef / h_elem);
}

// Compute temperature jump and flux average/jump
void
ADTestPeriodicSole::precalculateMaterial()
{
  _temp_jump_global = (_u_primary[_qp] - _u_secondary[_qp]);
}
