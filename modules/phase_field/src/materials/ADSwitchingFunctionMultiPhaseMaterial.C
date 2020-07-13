//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSwitchingFunctionMultiPhaseMaterial.h"

registerMooseObject("PhaseFieldApp", ADSwitchingFunctionMultiPhaseMaterial);

InputParameters
ADSwitchingFunctionMultiPhaseMaterial::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "h_name", "Name of the switching function material property for the given phase");
  params.addRequiredCoupledVar("phase_etas", "Vector of order parameters for the given phase");
  params.addRequiredCoupledVar("all_etas", "Vector of all order parameters for all phases");
  params.addClassDescription("Calculates the switching function for a given phase for a "
                             "multi-phase, multi-order parameter model");
  return params;
}

ADSwitchingFunctionMultiPhaseMaterial::ADSwitchingFunctionMultiPhaseMaterial(
    const InputParameters & parameters)
  : ADMaterial(parameters),
    DerivativeMaterialPropertyNameInterface(),
    _h_name(getParam<MaterialPropertyName>("h_name")),
    _num_eta_p(coupledComponents("phase_etas")),
    _eta_p(_num_eta_p),
    _eta_p_names(_num_eta_p),
    _num_eta(coupledComponents("all_etas")),
    _eta(_num_eta),
    _eta_names(_num_eta),
    _is_p(_num_eta),
    _prop_h(declareADProperty<Real>(_h_name)),
    _prop_dh(_num_eta)
{
  // Fetch eta values and names for phase etas
  for (unsigned int i = 0; i < _num_eta_p; ++i)
  {
    _eta_p[i] = &adCoupledValue("phase_etas", i);
    _eta_p_names[i] = getVar("phase_etas", i)->name();
  }

  // Declare h derivative properties, fetch eta values for all eta
  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    _eta[i] = &adCoupledValue("all_etas", i);
    _eta_names[i] = getVar("all_etas", i)->name();
    _prop_dh[i] = &declareADProperty<Real>(derivativePropertyNameFirst(_h_name, _eta_names[i]));
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
ADSwitchingFunctionMultiPhaseMaterial::computeQpProperties()
{
  ADReal sum_p = 0.0;
  ADReal sum_all = 0.0;

  for (unsigned int i = 0; i < _num_eta_p; ++i)
    sum_p += (*_eta_p[i])[_qp] * (*_eta_p[i])[_qp];

  for (unsigned int i = 0; i < _num_eta; ++i)
    sum_all += (*_eta[i])[_qp] * (*_eta[i])[_qp];

  ADReal sum_notp = sum_all - sum_p;

  _prop_h[_qp] = sum_p / sum_all;

  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    // First derivatives
    if (_is_p[i])
      (*_prop_dh[i])[_qp] = 2.0 * (*_eta[i])[_qp] * sum_notp / (sum_all * sum_all);
    else
      (*_prop_dh[i])[_qp] = -2.0 * (*_eta[i])[_qp] * sum_p / (sum_all * sum_all);
  }
}
