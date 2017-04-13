/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSMultiPhaseConcentration.h"

template <>
InputParameters
validParams<KKSMultiPhaseConcentration>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("KKS multi-phase model kernel to enforce (c = h1*c1 + h2*c2 + h3*c3 "
                             "+.. The non-linear variable of this kernel is cn, the final phase "
                             "concenration in the list.");
  params.addRequiredCoupledVar(
      "cj", "Array of phase concentrations cj. Place in same order as hj_names!");
  params.addRequiredCoupledVar("c", "Physical concentration");
  params.addCoupledVar("etas", "Order parameters for all phases");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names", "Switching Function Materials that provide h(eta_1, eta_2,...)");
  return params;
}

// Phase interpolation func
KKSMultiPhaseConcentration::KKSMultiPhaseConcentration(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _num_j(coupledComponents("cj")),
    _cjs(_num_j),
    _cjs_var(_num_j),
    _k(-1),
    _c(coupledValue("c")),
    _c_var(coupled("c")),
    _hj_names(getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_hj(_hj_names.size()),
    _eta_names(coupledComponents("etas")),
    _eta_vars(coupledComponents("etas")),
    _prop_dhjdetai(_num_j)
{
  // Check to make sure the the number of hj's is the same as the number of cj's
  if (_num_j != _hj_names.size())
    mooseError("Need to pass in as many hj_names as cjs in KKSMultiPhaseConcentration", name());
  // Check to make sure the the number of etas is the same as the number of cj's
  if (_num_j != _eta_names.size())
    mooseError("Need to pass in as many etas as cjs in KKSMultiPhaseConcentration", name());

  if (_num_j == 0)
    mooseError("Need to supply at least 1 phase concentration cj in KKSMultiPhaseConcentration",
               name());

  // get order parameter names and variable indices
  for (unsigned int i = 0; i < _num_j; ++i)
  {
    _eta_names[i] = getVar("etas", i)->name();
    _eta_vars[i] = coupled("etas", i);
  }

  // Load concentration variables into the arrays
  for (unsigned int m = 0; m < _num_j; ++m)
  {
    _cjs[m] = &coupledValue("cj", m);
    _cjs_var[m] = coupled("cj", m);
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
KKSMultiPhaseConcentration::computeQpResidual()
{
  // R = sum_i (h_i * c_i) - c
  Real sum_ch = 0.0;
  for (unsigned int m = 0; m < _num_j; ++m)
    sum_ch += (*_cjs[m])[_qp] * (*_prop_hj[m])[_qp];

  return _test[_i][_qp] * (sum_ch - _c[_qp]);
}

Real
KKSMultiPhaseConcentration::computeQpJacobian()
{
  return _test[_i][_qp] * (*_prop_hj[_k])[_qp] * _phi[_j][_qp];
}

Real
KKSMultiPhaseConcentration::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var)
    return -_test[_i][_qp] * _phi[_j][_qp];

  for (unsigned int m = 0; m < _num_j; ++m)
    if (jvar == _cjs_var[m])
      return _test[_i][_qp] * (*_prop_hj[m])[_qp] * _phi[_j][_qp];

  for (unsigned int m = 0; m < _num_j; ++m)
    if (jvar == _eta_vars[m])
    {
      Real sum = 0.0;

      for (unsigned int n = 0; n < _num_j; ++n)
        sum += (*_prop_dhjdetai[n][m])[_qp] * (*_cjs[n])[_qp];

      return _test[_i][_qp] * sum * _phi[_j][_qp];
    }

  return 0.0;
}
