//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaskedExponential.h"

registerMooseObject("PhaseFieldApp", MaskedExponential);

InputParameters
MaskedExponential::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar("w", "Chemical potential for the defect species");
  params.addRequiredCoupledVar("T", "Temperature");
  params.addClassDescription(
      "Kernel to add dilute solution term to Poisson's equation for electrochemical sintering");
  params.addParam<MaterialPropertyName>(
      "mask", "hm", "Mask function that specifies where this kernel is active");
  params.addRequiredParam<MaterialPropertyName>("n_eq", "Equilibrium defect concentration");
  params.addRequiredParam<int>("species_charge", "Charge of species this kernel is being used for");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  params.deprecateCoupledVar("args", "coupled_variables", "02/27/2024");
  return params;
}

MaskedExponential::MaskedExponential(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _w_var(coupled("w")),
    _w(coupledValue("w")),
    _temp_name(coupledName("T", 0)),
    _temp_var(coupled("T")),
    _temp(coupledValue("T")),
    _mask(getMaterialProperty<Real>("mask")),
    _prop_dmaskdarg(_n_args),
    _n_eq(getMaterialProperty<Real>("n_eq")),
    _prop_dn_eqdT(getMaterialPropertyDerivative<Real>("n_eq", _temp_name)),
    _prop_dn_eqdarg(_n_args),
    _z(getParam<int>("species_charge")),
    _kB(8.617343e-5), // eV/K
    _e(1.0)           // To put energy units in eV
{
  // Get derivatives of mask and equilibrium defect concentration
  for (unsigned int i = 0; i < _n_args; ++i)
  {
    _prop_dmaskdarg[i] = &getMaterialPropertyDerivative<Real>("mask", i);
    _prop_dn_eqdarg[i] = &getMaterialPropertyDerivative<Real>("n_eq", i);
  }
}

void
MaskedExponential::initialSetup()
{
  validateNonlinearCoupling<Real>("mask");
  validateNonlinearCoupling<Real>("n_eq");
}

Real
MaskedExponential::computeQpResidual()
{
  return _mask[_qp] * _z * _e * _n_eq[_qp] * _test[_i][_qp] *
         std::exp((_w[_qp] - _z * _e * _u[_qp]) / _kB / _temp[_qp]);
}

Real
MaskedExponential::computeQpJacobian()
{
  return -_mask[_qp] * _z * _z * _e * _e / _kB / _temp[_qp] * _n_eq[_qp] * _test[_i][_qp] *
         _phi[_j][_qp] * std::exp((_w[_qp] - _z * _e * _u[_qp]) / _kB / _temp[_qp]);
}

Real
MaskedExponential::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Handle chemical potential explicitly since it appears in the residual
  if (jvar == _w_var)
    return _mask[_qp] * _z * _e * _n_eq[_qp] / _kB / _temp[_qp] *
           std::exp((_w[_qp] - _z * _e * _u[_qp]) / _kB / _temp[_qp]) * _phi[_j][_qp] *
           _test[_i][_qp];

  // Handle temperature explicitly since it appears in the residual
  if (jvar == _temp_var)
    return _mask[_qp] * _z * _e * std::exp((_w[_qp] - _z * _e * _u[_qp]) / _kB / _temp[_qp]) *
           (_prop_dn_eqdT[_qp] -
            _n_eq[_qp] * (_w[_qp] - _z * _e * _u[_qp]) / _kB / _temp[_qp] / _temp[_qp]) *
           _phi[_j][_qp] * _test[_i][_qp];

  // General expression for remaining variable dependencies that don't appear in the residual
  //  for all other vars get the coupled variable jvar is referring to
  const unsigned int cvar = mapJvarToCvar(jvar);

  return _z * _e * std::exp((_w[_qp] - _z * _e * _u[_qp]) / _kB / _temp[_qp]) *
         ((*_prop_dmaskdarg[cvar])[_qp] + (*_prop_dn_eqdarg[cvar])[_qp]) * _test[_i][_qp] *
         _phi[_j][_qp];
}
