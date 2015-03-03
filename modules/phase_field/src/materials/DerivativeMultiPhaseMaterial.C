/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DerivativeMultiPhaseMaterial.h"

template<>
InputParameters validParams<DerivativeMultiPhaseMaterial>()
{
  InputParameters params = validParams<DerivativeMultiPhaseBase>();
  params.addClassDescription("Two phase material that combines n phase materials using a switching function with and n nonconserved order parameters (to be used with SwitchingFunctionConstraint*).");
  params.addCoupledVar("etas", "Order parameters for all phases.");
  return params;
}

DerivativeMultiPhaseMaterial::DerivativeMultiPhaseMaterial(const std::string & name,
                                                                               InputParameters parameters) :
    DerivativeMultiPhaseBase(name, parameters),
    _dhi(_num_etas),
    _d2hi(_num_etas)
{
  // verify that the user supplied one less eta than the number of phases
  if (_num_hi != _num_etas)
    mooseError("The number of coupled etas must be equal to the number of hi_names in DerivativeMultiPhaseMaterial " << name);

  for (unsigned int i = 0; i < _num_etas; ++i)
  {
    _dhi[i] = &getMaterialPropertyDerivative<Real>(_hi_names[i], _eta_names[i]);
    _d2hi[i] = &getMaterialPropertyDerivative<Real>(_hi_names[i], _eta_names[i], _eta_names[i]);
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

  if (i_eta >= 0 && j_eta >= 0)
  {
    // if the derivatives are taken w.r.t. a single eta the d2hi term for eta_i appears, otherwise it drops out
    // because we assume that hi _only_ depends on eta_i
    Real d2F = (i_eta == j_eta) ? (*_d2hi[i_eta])[_qp] * (*_prop_Fi[i_eta])[_qp] : 0.0;

    return  d2F + _W * (*_d2g[i_eta][j_eta])[_qp];
  }

  if (i_eta >= 0)
    return (*_dhi[i_eta])[_qp] * (*_prop_dFi[i_eta][j])[_qp];

  if (j_eta >= 0)
    return (*_dhi[j_eta])[_qp] * (*_prop_dFi[j_eta][i])[_qp];

  Real d2F = 0.0;
  for (unsigned n = 0; n < _num_fi; ++n)
    d2F += (*_hi[n])[_qp] * (*_prop_d2Fi[n][i][j])[_qp];
  return d2F;
}
