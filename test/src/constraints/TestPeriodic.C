//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestPeriodic.h"

namespace
{
const InputParameters &
setScalarParam(const InputParameters & params_in)
{
  // Reset the scalar_variable parameter to a relevant name for this physics
  InputParameters & ret = const_cast<InputParameters &>(params_in);
  ret.set<VariableName>("scalar_variable") = {
      params_in.get<VariableName>("kappa")};
  return ret;
}
}

registerMooseObject("MooseTestApp", TestPeriodic);

InputParameters
TestPeriodic::validParams()
{
  InputParameters params = MortarScalarBase::validParams();
  params.addRequiredParam<VariableName>("kappa", "Primary coupled scalar variable");
  params.addRequiredCoupledVar("kappa_aux", "Controlled scalar averaging variable");
  params.addParam<Real>("pen_scale", 1.0, "Increase or decrease the penalty");

  return params;
}

TestPeriodic::TestPeriodic(const InputParameters & parameters)
  : DerivativeMaterialInterface<MortarScalarBase>(setScalarParam(parameters)),
    _temp_jump_global(),
    _tau_s(),
    _kappa_aux_var(coupledScalar("kappa_aux")),
    _ka_order(getScalarVar("kappa_aux", 0)->order()),
    _kappa_aux(coupledScalarValue("kappa_aux")),
    _current_elem_volume(_assembly.elemVolume()),
    _current_side_volume(_assembly.sideElemVolume()),
    _pen_scale(getParam<Real>("pen_scale"))
{
}

// Compute the stability parameters to use for all quadrature points
void
TestPeriodic::precalculateResidual()
{
  precalculateStability();
}
void
TestPeriodic::precalculateJacobian()
{
  precalculateStability();
}
void
// Compute the temperature jump for current quadrature point
TestPeriodic::initScalarQpResidual()
{
  precalculateMaterial();
}

Real
TestPeriodic::computeQpResidual(const Moose::MortarType mortar_type)
{

  /// Compute penalty parameter times x-jump times average heat flux

  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  RealVectorValue kappa_vec(_kappa[0], _kappa[1], 0);
  Real r = (_pen_scale * _tau_s) * (kappa_vec * dx);
  // Real r = (_pen_scale * pencoef / h_elem) * ((_phys_points_secondary[_qp](0) -
  // _phys_points_primary[_qp](0))*_kappa[0]
  //               + (_phys_points_secondary[_qp](1) - _phys_points_primary[_qp](1))*_kappa[1]
  //               + (_phys_points_secondary[_qp](2) - _phys_points_primary[_qp](2))*_kappa[2]);

  switch (mortar_type)
  {
    // comment
    case Moose::MortarType::Secondary:
      // if (_i == 0)
      //   std::cout << "r-second" << r << std::endl;
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
TestPeriodic::computeScalarQpResidual()
{

  /// Stability/penalty term for residual of scalar variable
  Real r = (_pen_scale * _tau_s) * _temp_jump_global;
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);

  r *= -dx(_h);

  RealVectorValue kappa_vec(_kappa[0], _kappa[1], 0);
  RealVectorValue kappa_aux_vec(_kappa_aux[0], _kappa_aux[1], 0);

  r += dx(_h) * (_pen_scale * _tau_s) * (kappa_vec * dx);
  r -= dx(_h) * (kappa_aux_vec * _normals[_qp]);

  return r ;
}

Real
TestPeriodic::computeScalarQpJacobian()
{

  /// Stability/penalty term for Jacobian of scalar variable
  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);

  Real jac = dx(_h) * (_pen_scale * _tau_s) * dx(_l);

  return jac;
}

Real
TestPeriodic::computeQpOffDiagJacobianScalar(const Moose::MortarType mortar_type, const unsigned int svar_num)
{
  if (svar_num != _kappa_var)
    return 0;

  /// Stability/penalty term for Jacobian

  RealVectorValue dx(_phys_points_primary[_qp] - _phys_points_secondary[_qp]);
  Real jac = (_pen_scale * _tau_s);

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
TestPeriodic::computeScalarQpOffDiagJacobian(const Moose::MortarType mortar_type, const unsigned int jvar_num)
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
  Real jac = (_pen_scale * _tau_s);

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
TestPeriodic::precalculateStability()
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
TestPeriodic::precalculateMaterial()
{
  _temp_jump_global = (_u_primary[_qp] - _u_secondary[_qp]);
}
