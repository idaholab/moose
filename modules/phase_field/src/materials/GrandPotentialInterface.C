//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrandPotentialInterface.h"
#include "Conversion.h"
#include "IndirectSort.h"
#include "libmesh/utility.h"

registerMooseObject("PhaseFieldApp", GrandPotentialInterface);

InputParameters
GrandPotentialInterface::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Calculate Grand Potential interface parameters for a specified "
                             "interfacial free energy and width");
  params.addRequiredParam<std::vector<Real>>("sigma", "Interfacial free energies");
  params.addRequiredRangeCheckedParam<Real>(
      "width", "width > 0", "Interfacial width (for the interface with gamma = 1.5)");
  params.addParam<std::vector<MaterialPropertyName>>(
      "gamma_names",
      "Interfacial / grain boundary gamma parameter names (leave empty for gamma0... gammaN)");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa", "Gradient interface parameter name");
  params.addParam<MaterialPropertyName>("mu_name", "mu", "Grain growth bulk energy parameter name");
  params.addParam<unsigned int>(
      "sigma_index",
      "Sigma index to choose gamma = 1.5 for. Omit this to automatically chose the median sigma.");
  params.addParamNamesToGroup("mu_name sigma_index", "Advanced");
  return params;
}

GrandPotentialInterface::GrandPotentialInterface(const InputParameters & parameters)
  : Material(parameters),
    _sigma(getParam<std::vector<Real>>("sigma")),
    _width(getParam<Real>("width")),
    _n_pair(_sigma.size()),
    _gamma(_n_pair),
    _gamma_name(getParam<std::vector<MaterialPropertyName>>("gamma_names")),
    _gamma_prop(_n_pair),
    _kappa_prop(declareProperty<Real>(getParam<MaterialPropertyName>("kappa_name"))),
    _mu_prop(declareProperty<Real>(getParam<MaterialPropertyName>("mu_name")))
{
  // error check parameters
  if (_n_pair == 0)
    paramError("sigma", "Specify at least one interfacial energy");

  if (_gamma_name.size() != 0 && _gamma_name.size() != _n_pair)
    paramError("gamma_names",
               "Specify either as many entries are sigma values or none at all for auto-naming the "
               "gamma material properties.");

  // automatic names for the gamma properties
  if (_gamma_name.size() == 0)
    for (unsigned int i = 0; i < _n_pair; i++)
      _gamma_name[i] = "gamma" + Moose::stringify(i);

  // declare gamma material properties
  for (unsigned int i = 0; i < _n_pair; i++)
    _gamma_prop[i] = &declareProperty<Real>(_gamma_name[i]);

  // determine median interfacial free energy (or use explicit user choice)
  unsigned int median;
  if (isParamValid("sigma_index"))
    median = getParam<unsigned int>("sigma_index");
  else
  {
    std::vector<size_t> indices;
    Moose::indirectSort(_sigma.begin(), _sigma.end(), indices);
    median = indices[(indices.size() - 1) / 2];
  }

  // set the median gamma to 1.5 and use analytical expression for kappa and mu (m)
  _gamma[median] = 1.5;
  _kappa = 3.0 / 4.0 * _sigma[median] * _width;
  _mu = 6.0 * _sigma[median] / _width;

  // set all other gammas
  for (unsigned int i = 0; i < _n_pair; ++i)
  {
    // skip the already calculated median value
    if (i == median)
      continue;

    const Real g = _sigma[i] / std::sqrt(_mu * _kappa);

    // estimate for gamma from polynomial expansion
    Real gamma = 1.0 / (-5.288 * Utility::pow<8>(g) - 0.09364 * Utility::pow<6>(g) +
                        9.965 * Utility::pow<4>(g) - 8.183 * Utility::pow<2>(g) + 2.007);

    _gamma[i] = gamma;
  }
}

void
GrandPotentialInterface::computeQpProperties()
{
  _kappa_prop[_qp] = _kappa;
  _mu_prop[_qp] = _mu;
  for (unsigned int i = 0; i < _n_pair; ++i)
    (*_gamma_prop[i])[_qp] = _gamma[i];
}
