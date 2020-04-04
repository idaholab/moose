//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DerivativeMultiPhaseMaterial.h"

registerMooseObject("PhaseFieldApp", DerivativeMultiPhaseMaterial);

InputParameters
DerivativeMultiPhaseMaterial::validParams()
{
  InputParameters params = DerivativeMultiPhaseBase::validParams();
  params.addClassDescription("Two phase material that combines n phase materials using a switching "
                             "function with and n non-conserved order parameters (to be used with "
                             "SwitchingFunctionConstraint*).");
  params.addCoupledVar("etas", "Order parameters for all phases.");
  return params;
}

DerivativeMultiPhaseMaterial::DerivativeMultiPhaseMaterial(const InputParameters & parameters)
  : DerivativeMultiPhaseBase(parameters), _dhi(_num_etas), _d2hi(_num_etas), _d3hi(_num_etas)
{
  // verify that the user supplied one less eta than the number of phases
  if (_num_hi != _num_etas)
    paramError("hi_names", "The number of hi_names must be equal to the number of coupled etas");

  for (unsigned int i = 0; i < _num_etas; ++i)
  {
    _dhi[i] = &getMaterialPropertyDerivative<Real>(_hi_names[i], _eta_names[i]);
    _d2hi[i] = &getMaterialPropertyDerivative<Real>(_hi_names[i], _eta_names[i], _eta_names[i]);

    if (_third_derivatives)
      _d3hi[i] = &getMaterialPropertyDerivative<Real>(
          _hi_names[i], _eta_names[i], _eta_names[i], _eta_names[i]);
  }
}

Real
DerivativeMultiPhaseMaterial::computeDF(unsigned int i_var)
{
  const unsigned int i = argIndex(i_var);
  const int i_eta = _eta_index[i];

  if (i_eta >= 0)
    return (*_dhi[i_eta])[_qp] * (*_prop_Fi[i_eta])[_qp] + _W * (*_dg[i_eta])[_qp];
  else
  {
    Real dF = 0.0;
    for (unsigned n = 0; n < _num_fi; ++n)
      dF += (*_hi[n])[_qp] * (*_prop_dFi[n][i])[_qp];
    return dF;
  }
}

Real
DerivativeMultiPhaseMaterial::computeD2F(unsigned int i_var, unsigned int j_var)
{
  const unsigned int i = argIndex(i_var);
  const int i_eta = _eta_index[i];
  const unsigned int j = argIndex(j_var);
  const int j_eta = _eta_index[j];

  // all arguments are eta-variables

  if (i_eta >= 0 && j_eta >= 0)
  {
    // if the derivatives are taken w.r.t. a single eta the d2hi term for eta_i appears, otherwise
    // it drops out
    // because we assume that hi _only_ depends on eta_i
    Real d2F = (i_eta == j_eta) ? (*_d2hi[i_eta])[_qp] * (*_prop_Fi[i_eta])[_qp] : 0.0;

    return d2F + _W * (*_d2g[i_eta][j_eta])[_qp];
  }

  // one argument is an eta-variable

  if (i_eta >= 0)
    return (*_dhi[i_eta])[_qp] * (*_prop_dFi[i_eta][j])[_qp];

  if (j_eta >= 0)
    return (*_dhi[j_eta])[_qp] * (*_prop_dFi[j_eta][i])[_qp];

  // no arguments are eta-variables

  Real d2F = 0.0;
  for (unsigned n = 0; n < _num_fi; ++n)
    d2F += (*_hi[n])[_qp] * (*_prop_d2Fi[n][i][j])[_qp];
  return d2F;
}

Real
DerivativeMultiPhaseMaterial::computeD3F(unsigned int i_var, unsigned int j_var, unsigned int k_var)
{
  const unsigned int i = argIndex(i_var);
  const int i_eta = _eta_index[i];
  const unsigned int j = argIndex(j_var);
  const int j_eta = _eta_index[j];
  const unsigned int k = argIndex(k_var);
  const int k_eta = _eta_index[k];

  // all arguments are eta-variables

  if (i_eta >= 0 && j_eta >= 0 && k_eta >= 0)
  {
    // if the derivatives are taken w.r.t. a single eta the d3hi term for eta_i appears, otherwise
    // it drops out
    // because we assume that hi _only_ depends on eta_i
    Real d3F =
        (i_eta == j_eta && j_eta == k_eta) ? (*_d3hi[i_eta])[_qp] * (*_prop_Fi[i_eta])[_qp] : 0.0;

    return d3F + _W * (*_d3g[i_eta][j_eta][k_eta])[_qp];
  }

  // two arguments are eta-variables

  if (i_eta >= 0 && j_eta >= 0)
    return (i_eta == j_eta) ? (*_d2hi[i_eta])[_qp] * (*_prop_dFi[i_eta][k])[_qp] : 0.0;

  if (j_eta >= 0 && k_eta >= 0)
    return (j_eta == k_eta) ? (*_d2hi[j_eta])[_qp] * (*_prop_dFi[j_eta][i])[_qp] : 0.0;

  if (k_eta >= 0 && i_eta >= 0)
    return (k_eta == i_eta) ? (*_d2hi[k_eta])[_qp] * (*_prop_dFi[k_eta][j])[_qp] : 0.0;

  // one argument is an eta-variable

  if (i_eta >= 0)
    return (*_dhi[i_eta])[_qp] * (*_prop_d2Fi[i_eta][j][k])[_qp];

  if (j_eta >= 0)
    return (*_dhi[j_eta])[_qp] * (*_prop_d2Fi[j_eta][i][k])[_qp];

  if (k_eta >= 0)
    return (*_dhi[k_eta])[_qp] * (*_prop_d2Fi[k_eta][i][j])[_qp];

  // no arguments are eta-variables

  Real d3F = 0.0;
  for (unsigned n = 0; n < _num_fi; ++n)
    d3F += (*_hi[n])[_qp] * (*_prop_d3Fi[n][i][j][k])[_qp];
  return d3F;
}
