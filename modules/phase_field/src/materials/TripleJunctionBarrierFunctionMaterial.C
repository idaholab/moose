//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TripleJunctionBarrierFunctionMaterial.h"

registerMooseObject("PhaseFieldApp", TripleJunctionBarrierFunctionMaterial);

InputParameters
TripleJunctionBarrierFunctionMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("function_name", "f_obs", "actual name for f_obs(eta)");
  params.addRequiredCoupledVar("etas", "eta_i order parameters for all phases");
  params.addParam<Real>("h",
                        0.0,
                        "Uniform obstacle coefficient applied to every i<j<k triple when h_ijk "
                        "is not supplied");
  params.addParam<std::vector<Real>>(
      "h_ijk",
      {},
      "Flat list of per-triple obstacle coefficients in lexicographic i<j<k order. Must have "
      "exactly n*(n-1)*(n-2)/6 entries for n coupled etas. Overrides 'h' when supplied.");
  params.addClassDescription(
      "Obstacle-type free energy contribution that suppresses spurious nucleation of a fourth "
      "phase at triple junctions: $f_{obs} = \\sum_{i<j<k} h_{ijk} \\eta_i^2 \\eta_j^2 \\eta_k^2$. "
      "See Kundin, Pogorelov, and Emmerich, Acta Mater., v. 83, p. 448-459 (2015).");
  return params;
}

TripleJunctionBarrierFunctionMaterial::TripleJunctionBarrierFunctionMaterial(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _function_name(getParam<std::string>("function_name")),
    _num_eta(coupledComponents("etas")),
    _eta_names(coupledNames("etas")),
    _eta(coupledValues("etas")),
    _h(getParam<Real>("h")),
    _h_ijk(getParam<std::vector<Real>>("h_ijk")),
    _prop_g(declareProperty<Real>(_function_name)),
    _prop_dg(_num_eta),
    _prop_d2g(_num_eta)
{
  if (_num_eta < 3)
    paramError("etas",
               "TripleJunctionBarrierFunctionMaterial requires at least 3 order parameters");

  const unsigned int num_triples = _num_eta * (_num_eta - 1) * (_num_eta - 2) / 6;
  if (!_h_ijk.empty() && _h_ijk.size() != num_triples)
    paramError("h_ijk",
               "Size of h_ijk does not match the number of i<j<k triples of the coupled etas. "
               "Supply h_ijk of correct size, or omit it to use the uniform 'h' coefficient.");

  for (const auto i : make_range(_num_eta))
    _prop_d2g[i].resize(_num_eta, NULL);

  for (const auto i : make_range(_num_eta))
  {
    _prop_dg[i] = &declarePropertyDerivative<Real>(_function_name, _eta_names[i]);
    for (const auto j : make_range(i, _num_eta))
      _prop_d2g[i][j] = _prop_d2g[j][i] =
          &declarePropertyDerivative<Real>(_function_name, _eta_names[i], _eta_names[j]);
  }
}

void
TripleJunctionBarrierFunctionMaterial::computeQpProperties()
{
  // Initialize properties to zero before accumulating over triples
  _prop_g[_qp] = 0.0;
  for (const auto i : make_range(_num_eta))
    for (const auto j : make_range(i, _num_eta))
      (*_prop_d2g[i][j])[_qp] = 0.0;
  for (const auto i : make_range(_num_eta))
    (*_prop_dg[i])[_qp] = 0.0;

  unsigned int idx = 0;
  for (const auto i : make_range(_num_eta))
    for (const auto j : make_range(i + 1, _num_eta))
      for (const auto k : make_range(j + 1, _num_eta))
      {
        const Real hijk = _h_ijk.empty() ? _h : _h_ijk[idx++];

        const Real ei = (*_eta[i])[_qp];
        const Real ej = (*_eta[j])[_qp];
        const Real ek = (*_eta[k])[_qp];
        const Real ei2 = ei * ei;
        const Real ej2 = ej * ej;
        const Real ek2 = ek * ek;

        _prop_g[_qp] += hijk * ei2 * ej2 * ek2;

        // first derivatives
        (*_prop_dg[i])[_qp] += 2.0 * hijk * ei * ej2 * ek2;
        (*_prop_dg[j])[_qp] += 2.0 * hijk * ei2 * ej * ek2;
        (*_prop_dg[k])[_qp] += 2.0 * hijk * ei2 * ej2 * ek;

        // second derivatives (diagonal)
        (*_prop_d2g[i][i])[_qp] += 2.0 * hijk * ej2 * ek2;
        (*_prop_d2g[j][j])[_qp] += 2.0 * hijk * ei2 * ek2;
        (*_prop_d2g[k][k])[_qp] += 2.0 * hijk * ei2 * ej2;

        // second derivatives (off-diagonal) -- accumulate: an unordered pair can appear in
        // more than one triple (once per distinct third index)
        (*_prop_d2g[i][j])[_qp] += 4.0 * hijk * ei * ej * ek2;
        (*_prop_d2g[i][k])[_qp] += 4.0 * hijk * ei * ek * ej2;
        (*_prop_d2g[j][k])[_qp] += 4.0 * hijk * ej * ek * ei2;
      }
}
