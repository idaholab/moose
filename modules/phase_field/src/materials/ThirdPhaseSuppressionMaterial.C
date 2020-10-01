//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThirdPhaseSuppressionMaterial.h"

registerMooseObject("PhaseFieldApp", ThirdPhaseSuppressionMaterial);

InputParameters
ThirdPhaseSuppressionMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("function_name", "g", "actual name for g(eta_i)");
  params.addRequiredCoupledVar("etas", "eta_i order parameters, one for each h");
  params.addClassDescription(
      "Free Energy contribution that penalizes more than two order parameters being non-zero");
  return params;
}

ThirdPhaseSuppressionMaterial::ThirdPhaseSuppressionMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _function_name(getParam<std::string>("function_name")),
    _num_eta(coupledComponents("etas")),
    _eta(coupledValues("etas")),
    _prop_g(declareProperty<Real>(_function_name)),
    _prop_dg(_num_eta),
    _prop_d2g(_num_eta)
{
  const auto eta_name = coupledNames("etas");

  for (unsigned int i = 0; i < _num_eta; ++i)
    _prop_d2g[i].resize(_num_eta);

  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    _prop_dg[i] = &declarePropertyDerivative<Real>(_function_name, eta_name[i]);
    for (unsigned int j = i; j < _num_eta; ++j)
    {
      _prop_d2g[i][j] = _prop_d2g[j][i] =
          &declarePropertyDerivative<Real>(_function_name, eta_name[i], eta_name[j]);
    }
  }
}

void
ThirdPhaseSuppressionMaterial::computeQpProperties()
{
  // Initialize properties to zero before accumulating
  _prop_g[_qp] = 0.0;
  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    (*_prop_dg[i])[_qp] = 0.0;
    for (unsigned int j = i; j < _num_eta; ++j)
      (*_prop_d2g[i][j])[_qp] = 0.0;
  }

  // Create Interface barrier preventing interfaces involving more than two order parameters
  for (unsigned int i = 0; i < _num_eta; ++i)
    for (unsigned int j = 0; j < i; ++j)
      for (unsigned int k = 0; k < j; ++k)
      {
        const Real ni = (*_eta[i])[_qp];
        const Real nj = (*_eta[j])[_qp];
        const Real nk = (*_eta[k])[_qp];

        _prop_g[_qp] += ni * ni * nj * nj * nk * nk;
        (*_prop_dg[i])[_qp] += 2 * ni * nj * nj * nk * nk;
        (*_prop_dg[j])[_qp] += 2 * ni * ni * nj * nk * nk;
        (*_prop_dg[k])[_qp] += 2 * ni * ni * nj * nj * nk;
        (*_prop_d2g[i][i])[_qp] += 2 * nj * nj * nk * nk;
        (*_prop_d2g[j][j])[_qp] += 2 * ni * ni * nk * nk;
        (*_prop_d2g[k][k])[_qp] += 2 * ni * ni * nj * nj;
        (*_prop_d2g[i][j])[_qp] += 4 * ni * nj * nk * nk;
        (*_prop_d2g[i][k])[_qp] += 4 * ni * nj * nj * nk;
        (*_prop_d2g[k][j])[_qp] += 4 * ni * ni * nj * nk;
      }
}
