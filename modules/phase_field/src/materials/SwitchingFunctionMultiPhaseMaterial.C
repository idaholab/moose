/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SwitchingFunctionMultiPhaseMaterial.h"

template <>
InputParameters
validParams<SwitchingFunctionMultiPhaseMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<MaterialPropertyName>(
      "h_name", "Name of the switching function material property for the given phase");
  params.addRequiredCoupledVar("phase_etas", "Vector of order parameters for the given phase");
  params.addRequiredCoupledVar("all_etas", "Vector of all order parameters for all phases");
  params.addClassDescription("Calculates the switching function for a given phase for a "
                             "multi-phase, multi-order parameter model");
  return params;
}

SwitchingFunctionMultiPhaseMaterial::SwitchingFunctionMultiPhaseMaterial(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _h_name(getParam<MaterialPropertyName>("h_name")),
    _num_eta_p(coupledComponents("phase_etas")),
    _eta_p(_num_eta_p),
    _eta_p_names(_num_eta_p),
    _num_eta(coupledComponents("all_etas")),
    _eta(_num_eta),
    _eta_names(_num_eta),
    _is_p(_num_eta),
    _prop_h(declareProperty<Real>(_h_name)),
    _prop_dh(_num_eta),
    _prop_d2h(_num_eta)
{
  // Fetch eta values and names for phase etas
  for (unsigned int i = 0; i < _num_eta_p; ++i)
  {
    _eta_p[i] = &coupledValue("phase_etas", i);
    _eta_p_names[i] = getVar("phase_etas", i)->name();
  }

  // Declare h derivative properties, fetch eta values for all eta
  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    _prop_d2h[i].resize(_num_eta);
    _eta_names[i] = getVar("all_etas", i)->name();
  }

  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    _prop_dh[i] = &declarePropertyDerivative<Real>(_h_name, _eta_names[i]);
    _eta[i] = &coupledValue("all_etas", i);
    for (unsigned int j = i; j < _num_eta; ++j)
    {
      _prop_d2h[i][j] = _prop_d2h[j][i] =
          &declarePropertyDerivative<Real>(_h_name, _eta_names[i], _eta_names[j]);
    }
  }

  // Determine which order parameters in the list of all etas belong to phase p
  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    _is_p[i] = false;
    for (unsigned int j = 0; j < _num_eta_p; ++j)
    {
      if (_eta_names[i] == _eta_p_names[j])
        _is_p[i] = true;
    }
  }
}

void
SwitchingFunctionMultiPhaseMaterial::computeQpProperties()
{
  Real sum_p = 0.0;
  Real sum_all = 0.0;

  for (unsigned int i = 0; i < _num_eta_p; ++i)
    sum_p += (*_eta_p[i])[_qp] * (*_eta_p[i])[_qp];

  for (unsigned int i = 0; i < _num_eta; ++i)
    sum_all += (*_eta[i])[_qp] * (*_eta[i])[_qp];

  Real sum_notp = sum_all - sum_p;

  _prop_h[_qp] = sum_p / sum_all;

  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    // First derivatives
    if (_is_p[i])
      (*_prop_dh[i])[_qp] = 2.0 * (*_eta[i])[_qp] * sum_notp / (sum_all * sum_all);
    else
      (*_prop_dh[i])[_qp] = -2.0 * (*_eta[i])[_qp] * sum_p / (sum_all * sum_all);

    // Second derivatives
    for (unsigned int j = 0; j < _num_eta; ++j)
    {
      if (i == j)
      {
        if (_is_p[i])
          (*_prop_d2h[i][j])[_qp] =
              (2.0 * sum_all * sum_notp - 8.0 * (*_eta[i])[_qp] * (*_eta[i])[_qp] * sum_notp) /
              (sum_all * sum_all * sum_all);
        else
          (*_prop_d2h[i][j])[_qp] =
              (-2.0 * sum_p * sum_all + 8.0 * (*_eta[i])[_qp] * (*_eta[i])[_qp] * sum_p) /
              (sum_all * sum_all * sum_all);
      }
      else if (_is_p[i] && _is_p[j])
        (*_prop_d2h[i][j])[_qp] =
            -8.0 * (*_eta[i])[_qp] * (*_eta[j])[_qp] * sum_notp / (sum_all * sum_all * sum_all);
      else if (!_is_p[i] && !_is_p[j])
        (*_prop_d2h[i][j])[_qp] =
            8.0 * (*_eta[i])[_qp] * (*_eta[j])[_qp] * sum_p / (sum_all * sum_all * sum_all);
      else
        (*_prop_d2h[i][j])[_qp] = (4.0 * sum_all - 8.0 * sum_notp) * (*_eta[i])[_qp] *
                                  (*_eta[j])[_qp] / (sum_all * sum_all * sum_all);
    }
  }
}
