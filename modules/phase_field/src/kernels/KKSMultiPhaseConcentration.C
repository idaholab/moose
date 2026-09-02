//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KKSMultiPhaseConcentration.h"

registerMooseObject("PhaseFieldApp", KKSMultiPhaseConcentration);
registerMooseObject("PhaseFieldApp", ADKKSMultiPhaseConcentration);

template <bool is_ad>
InputParameters
KKSMultiPhaseConcentrationTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addClassDescription(
      "KKS multiphase kernel enforcing $c = h_1c_1 + h_2c_2 + h_3c_3 + \\dots$. "
      "The nonlinear variable must be one of the phase concentrations listed in cj.");
  params.addRequiredCoupledVar("cj", "Phase concentrations c_j, in the same order as hj_names");
  params.addRequiredCoupledVar("c", "Physical concentration");
  params.addRequiredCoupledVar("etas", "Order parameters, one for each phase");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "hj_names", "Switching functions h_j, in the same order as cj and etas");
  return params;
}

template <bool is_ad>
KKSMultiPhaseConcentrationTempl<is_ad>::KKSMultiPhaseConcentrationTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<GenericKernel<is_ad>>>(parameters),
    _num_j(this->coupledComponents("cj")),
    _cj(this->template coupledGenericValues<is_ad>("cj")),
    _k(-1),
    _c(this->template coupledGenericValue<is_ad>("c")),
    _hj_names(this->template getParam<std::vector<MaterialPropertyName>>("hj_names")),
    _prop_hj(_num_j),
    _eta_names(this->coupledComponents("etas"))
{
  if (_num_j == 0)
    this->paramError("cj", "At least one phase concentration must be supplied");

  if (_hj_names.size() != _num_j)
    this->paramError("hj_names", "The number of switching functions must equal the number of cjs");

  if (_eta_names.size() != _num_j)
    this->paramError("etas", "The number of order parameters must equal the number of cjs");

  for (unsigned int j = 0; j < _num_j; ++j)
  {
    _eta_names[j] = this->coupledName("etas", j);
    _prop_hj[j] = &this->template getGenericMaterialPropertyByName<Real, is_ad>(_hj_names[j]);

    if (this->coupled("cj", j) == _var.number())
      _k = j;
  }

  if (_k < 0)
    this->paramError("cj", "The kernel variable must be one of the phase concentrations in cj");
}

template <bool is_ad>
GenericReal<is_ad>
KKSMultiPhaseConcentrationTempl<is_ad>::computeQpResidual()
{
  GenericReal<is_ad> sum_ch = 0.0;
  for (unsigned int j = 0; j < _num_j; ++j)
    sum_ch += (*_cj[j])[_qp] * (*_prop_hj[j])[_qp];

  return _test[_i][_qp] * (sum_ch - _c[_qp]);
}

KKSMultiPhaseConcentration::KKSMultiPhaseConcentration(const InputParameters & parameters)
  : KKSMultiPhaseConcentrationTempl<false>(parameters),
    _cj_map(getParameterJvarMap("cj")),
    _c_var(coupled("c")),
    _eta_map(getParameterJvarMap("etas")),
    _prop_dhjdetai(_num_j, std::vector<const MaterialProperty<Real> *>(_num_j, nullptr))
{
  for (unsigned int j = 0; j < _num_j; ++j)
    for (unsigned int i = 0; i < _num_j; ++i)
      _prop_dhjdetai[j][i] =
          &getMaterialPropertyDerivativeByName<Real>(_hj_names[j], _eta_names[i]);
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

  const int cjvar = mapJvarToCvar(jvar, _cj_map);
  if (cjvar >= 0)
    return _test[_i][_qp] * (*_prop_hj[cjvar])[_qp] * _phi[_j][_qp];

  const int etavar = mapJvarToCvar(jvar, _eta_map);
  if (etavar >= 0)
  {
    Real sum = 0.0;
    for (unsigned int j = 0; j < _num_j; ++j)
      sum += (*_prop_dhjdetai[j][etavar])[_qp] * (*_cj[j])[_qp];

    return _test[_i][_qp] * sum * _phi[_j][_qp];
  }

  return 0.0;
}

template class KKSMultiPhaseConcentrationTempl<false>;
template class KKSMultiPhaseConcentrationTempl<true>;