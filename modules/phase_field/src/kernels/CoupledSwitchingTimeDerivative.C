//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledSwitchingTimeDerivative.h"

registerMooseObject("PhaseFieldApp", CoupledSwitchingTimeDerivative);

InputParameters
CoupledSwitchingTimeDerivative::validParams()
{
  InputParameters params = CoupledTimeDerivative::validParams();
  params.addClassDescription(
      "Coupled time derivative Kernel that multiplies the time derivative by "
      "$\\frac{dh_\\alpha}{d\\eta_i} F_\\alpha + \\frac{dh_\\beta}{d\\eta_i} F_\\beta + \\dots$");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "Fj_names", "List of functions for each phase. Place in same order as hj_names!");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names", "Switching Function Materials that provide h. Place in same order as Fj_names!");
  params.addCoupledVar("args", "Vector of variable arguments of Fj and hj");
  params.deprecateCoupledVar("args", "coupled_variables", "02/27/2024");
  return params;
}

CoupledSwitchingTimeDerivative::CoupledSwitchingTimeDerivative(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<CoupledTimeDerivative>>(parameters),
    _v_name(coupledName("v", 0)),
    _Fj_names(getParam<std::vector<MaterialPropertyName>>("Fj_names")),
    _num_j(_Fj_names.size()),
    _prop_Fj(_num_j),
    _prop_dFjdv(_num_j),
    _prop_dFjdarg(_num_j),
    _hj_names(getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_dhjdetai(_num_j),
    _prop_d2hjdetai2(_num_j),
    _prop_d2hjdetaidarg(_num_j)
{
  // check passed in parameter vectors
  if (_num_j != _hj_names.size())
    paramError("hj_names", "Need to pass in as many hj_names as Fj_names");

  // reserve space and set phase material properties
  for (unsigned int n = 0; n < _num_j; ++n)
  {
    // get phase free energy and derivatives
    _prop_Fj[n] = &getMaterialPropertyByName<Real>(_Fj_names[n]);
    _prop_dFjdv[n] = &getMaterialPropertyDerivative<Real>(_Fj_names[n], _var.name());
    _prop_dFjdarg[n].resize(_n_args);

    // get switching function and derivatives wrt eta_i, the nonlinear variable
    _prop_dhjdetai[n] = &getMaterialPropertyDerivative<Real>(_hj_names[n], _v_name);
    _prop_d2hjdetai2[n] = &getMaterialPropertyDerivative<Real>(_hj_names[n], _v_name, _v_name);
    _prop_d2hjdetaidarg[n].resize(_n_args);

    for (unsigned int i = 0; i < _n_args; ++i)
    {
      // Get derivatives of all Fj wrt all coupled variables
      _prop_dFjdarg[n][i] = &getMaterialPropertyDerivative<Real>(_Fj_names[n], i);

      // Get second derivatives of all hj wrt eta_i and all coupled variables
      _prop_d2hjdetaidarg[n][i] = &getMaterialPropertyDerivative<Real>(_hj_names[n], _v_name, i);
    }
  }
}

void
CoupledSwitchingTimeDerivative::initialSetup()
{
  for (unsigned int n = 0; n < _num_j; ++n)
  {
    validateNonlinearCoupling<Real>(_Fj_names[n]);
    validateNonlinearCoupling<Real>(_hj_names[n]);
  }
}

Real
CoupledSwitchingTimeDerivative::computeQpResidual()
{
  Real sum = 0.0;
  for (unsigned int n = 0; n < _num_j; ++n)
    sum += (*_prop_dhjdetai[n])[_qp] * (*_prop_Fj[n])[_qp];

  return CoupledTimeDerivative::computeQpResidual() * sum;
}

Real
CoupledSwitchingTimeDerivative::computeQpJacobian()
{
  Real sum = 0.0;
  for (unsigned int n = 0; n < _num_j; ++n)
    sum += (*_prop_dhjdetai[n])[_qp] * (*_prop_dFjdv[n])[_qp];

  return CoupledTimeDerivative::computeQpResidual() * sum * _phi[_j][_qp];
}

Real
CoupledSwitchingTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  if (jvar == _v_var)
  {
    Real sum = 0.0;

    for (unsigned int n = 0; n < _num_j; ++n)
      sum +=
          ((*_prop_d2hjdetai2[n])[_qp] * _v_dot[_qp] + (*_prop_dhjdetai[n])[_qp] * _dv_dot[_qp]) *
          (*_prop_Fj[n])[_qp];

    return _phi[_j][_qp] * sum * _test[_i][_qp];
  }

  Real sum = 0.0;
  for (unsigned int n = 0; n < _num_j; ++n)
    sum += (*_prop_d2hjdetaidarg[n][cvar])[_qp] * (*_prop_Fj[n])[_qp] +
           (*_prop_dhjdetai[n])[_qp] * (*_prop_dFjdarg[n][cvar])[_qp];

  return CoupledTimeDerivative::computeQpResidual() * sum * _phi[_j][_qp];
}
