//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSMultiPhaseConcentration.h"

registerMooseObject("PhaseFieldApp", KKSMultiPhaseConcentration);

InputParameters
KKSMultiPhaseConcentration::validParams()
{
  InputParameters params = KernelValue::validParams();
  params.addClassDescription(
      "KKS multi-phase model kernel to enforce $c = h_1c_1 + h_2c_2 + h_3c_3 + \\dots$"
      ". The non-linear variable of this kernel is $c_n$, the final phase "
      "concentration in the list.");
  params.addRequiredCoupledVar(
      "cj", "Array of phase concentrations cj. Place in same order as hj_names!");
  params.addRequiredCoupledVar("c", "Physical concentration");
  params.addCoupledVar("etas", "Order parameters for all phases");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names", "Switching Function Materials that provide $h(\\eta_1, \\eta_2,\\dots)$");
  return params;
}

// Phase interpolation func
KKSMultiPhaseConcentration::KKSMultiPhaseConcentration(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<KernelValue>>(parameters),
    _num_j(coupledComponents("cj")),
    _cj(coupledValues("cj")),
    _cj_map(getParameterJvarMap("cj")),
    _k(-1),
    _c(coupledValue("c")),
    _c_var(coupled("c")),
    _hj_names(getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_hj(_hj_names.size()),
    _eta_names(coupledComponents("etas")),
    _eta_map(getParameterJvarMap("etas")),
    _prop_dhjdetai(_num_j)
{
  // Check to make sure the the number of hj's is the same as the number of cj's
  if (_num_j != _hj_names.size())
    paramError("hj_names", "Need to pass in as many hj_names as cjs");
  // Check to make sure the the number of etas is the same as the number of cj's
  if (_num_j != _eta_names.size())
    paramError("etas", "Need to pass in as many etas as cjs");

  if (_num_j == 0)
    mooseError("Need to supply at least 1 phase concentration cj in KKSMultiPhaseConcentration",
               name());

  // get order parameter names and variable indices
  for (unsigned int i = 0; i < _num_j; ++i)
    _eta_names[i] = coupledName("etas", i);

  // Load concentration variables into the arrays
  for (unsigned int m = 0; m < _num_j; ++m)
  {
    _prop_hj[m] = &getMaterialPropertyByName<Real>(_hj_names[m]);
    _prop_dhjdetai[m].resize(_num_j);
    // Set _k to the position of the nonlinear variable in the list of cj's
    if (coupled("cj", m) == _var.number())
      _k = m;

    // Get derivatives of switching functions wrt order parameters
    for (unsigned int n = 0; n < _num_j; ++n)
      _prop_dhjdetai[m][n] = &getMaterialPropertyDerivative<Real>(_hj_names[m], _eta_names[n]);
  }

  // Check to make sure the nonlinear variable is set to one of the cj's
  if (_k < 0)
    mooseError("Need to set nonlinear variable to one of the cj's in KKSMultiPhaseConcentration",
               name());
}

Real
KKSMultiPhaseConcentration::precomputeQpResidual()
{
  // R = sum_i (h_i * c_i) - c
  Real sum_ch = 0.0;
  for (unsigned int m = 0; m < _num_j; ++m)
    sum_ch += (*_cj[m])[_qp] * (*_prop_hj[m])[_qp];

  return sum_ch - _c[_qp];
}

Real
KKSMultiPhaseConcentration::precomputeQpJacobian()
{
  return (*_prop_hj[_k])[_qp] * _phi[_j][_qp];
}

Real
KKSMultiPhaseConcentration::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var)
    return -_test[_i][_qp] * _phi[_j][_qp];

  auto cjvar = mapJvarToCvar(jvar, _cj_map);
  if (cjvar >= 0)
    return _test[_i][_qp] * (*_prop_hj[cjvar])[_qp] * _phi[_j][_qp];

  auto etavar = mapJvarToCvar(jvar, _eta_map);
  if (etavar >= 0)
  {
    Real sum = 0.0;

    for (unsigned int n = 0; n < _num_j; ++n)
      sum += (*_prop_dhjdetai[n][etavar])[_qp] * (*_cj[n])[_qp];

    return _test[_i][_qp] * sum * _phi[_j][_qp];
  }

  return 0.0;
}
