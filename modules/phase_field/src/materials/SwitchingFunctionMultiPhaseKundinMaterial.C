//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SwitchingFunctionMultiPhaseKundinMaterial.h"

registerMooseObject("PhaseFieldApp", SwitchingFunctionMultiPhaseKundinMaterial);
registerMooseObject("PhaseFieldApp", ADSwitchingFunctionMultiPhaseKundinMaterial);

template <bool is_ad>
InputParameters
SwitchingFunctionMultiPhaseKundinMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "h_name", "Name of the switching function material property for the given phase");
  params.addRequiredCoupledVar("eta_i", "Order parameter for the given phase");
  params.addRequiredCoupledVar("all_etas", "Vector of all order parameters for all phases");
  params.addClassDescription(
      "Calculates the switching function for a given phase for a general multi-phase, "
      "multi-order parameter model with an arbitrary number of phases: "
      "$g_i = \\eta_i^2/2 [15 (1-\\eta_i) (1 - \\eta_i - \\sum_{j \\neq i} \\eta_j^2) + "
      "\\eta_i (5 - 3\\eta_i^2)]$. See Kundin, Pogorelov, and Emmerich, Acta Mater., "
      "v. 83, p. 448-459 (2015), Eq. (4).");
  return params;
}

template <bool is_ad>
SwitchingFunctionMultiPhaseKundinMaterialTempl<
    is_ad>::SwitchingFunctionMultiPhaseKundinMaterialTempl(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _h_name(this->getParam<MaterialPropertyName>("h_name")),
    _eta_i(coupledGenericValue<is_ad>("eta_i")),
    _eta_i_name(coupledName("eta_i")),
    _num_eta(coupledComponents("all_etas")),
    _eta(coupledGenericValues<is_ad>("all_etas")),
    _eta_names(coupledNames("all_etas")),
    _self_index(0),
    _prop_h(declareGenericProperty<Real, is_ad>(_h_name)),
    _prop_dh(_num_eta),
    _prop_d2h(_num_eta)
{
  // Find eta_i within the list of all etas
  bool found = false;
  for (const auto i : make_range(_num_eta))
    if (_eta_names[i] == _eta_i_name)
    {
      _self_index = i;
      found = true;
    }
  if (!found)
    paramError("eta_i", "eta_i must be a member of the all_etas coupled vector");

  // Declare h derivative properties
  for (const auto i : make_range(_num_eta))
    _prop_d2h[i].resize(_num_eta, NULL);

  for (const auto i : make_range(_num_eta))
  {
    _prop_dh[i] = &this->template declarePropertyDerivative<Real, is_ad>(_h_name, _eta_names[i]);
    for (const auto j : make_range(i, _num_eta))
      _prop_d2h[i][j] = _prop_d2h[j][i] = &this->template declarePropertyDerivative<Real, is_ad>(
          _h_name, _eta_names[i], _eta_names[j]);
  }
}

template <bool is_ad>
void
SwitchingFunctionMultiPhaseKundinMaterialTempl<is_ad>::computeQpProperties()
{
  const GenericReal<is_ad> e = _eta_i[_qp];

  GenericReal<is_ad> sum_all = 0.0;
  for (const auto i : make_range(_num_eta))
    sum_all += (*_eta[i])[_qp] * (*_eta[i])[_qp];

  // S = sum_{j != i} eta_j^2
  const GenericReal<is_ad> S = sum_all - e * e;

  // A = 15(1-e)^2 - 15(1-e)*S + 5e - 3e^3, so that g_i = e^2/2 * A
  const GenericReal<is_ad> A =
      15.0 * (1.0 - e) * (1.0 - e) - 15.0 * (1.0 - e) * S + 5.0 * e - 3.0 * e * e * e;
  // dA/de = -30(1-e) + 15S + 5 - 9e^2
  const GenericReal<is_ad> dAde = -30.0 * (1.0 - e) + 15.0 * S + 5.0 - 9.0 * e * e;

  _prop_h[_qp] = 0.5 * e * e * A;

  for (const auto i : make_range(_num_eta))
  {
    const GenericReal<is_ad> eta_i_val = (*_eta[i])[_qp];

    // First derivatives
    if (i == _self_index)
      (*_prop_dh[i])[_qp] = e * A + 0.5 * e * e * dAde;
    else
      (*_prop_dh[i])[_qp] = -15.0 * e * e * (1.0 - e) * eta_i_val;

    // Second derivatives
    for (const auto j : make_range(_num_eta))
    {
      const GenericReal<is_ad> eta_j_val = (*_eta[j])[_qp];

      if (i == _self_index && j == _self_index)
        (*_prop_d2h[i][j])[_qp] = A + 2.0 * e * dAde + 0.5 * e * e * (30.0 - 18.0 * e);
      else if (i == _self_index && j != _self_index)
        (*_prop_d2h[i][j])[_qp] = -15.0 * eta_j_val * e * (2.0 - 3.0 * e);
      else if (i != _self_index && j == _self_index)
        (*_prop_d2h[i][j])[_qp] = -15.0 * eta_i_val * e * (2.0 - 3.0 * e);
      else if (i == j)
        (*_prop_d2h[i][j])[_qp] = -15.0 * e * e * (1.0 - e);
      else
        (*_prop_d2h[i][j])[_qp] = 0.0;
    }
  }
}

// explicit instantiation
template class SwitchingFunctionMultiPhaseKundinMaterialTempl<true>;
template class SwitchingFunctionMultiPhaseKundinMaterialTempl<false>;
