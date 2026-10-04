//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrandPotentialInterface.h"
#include "Conversion.h"
#include "IndirectSort.h"
#include "MoelansInterfaceFits.h"

#include <algorithm>

registerMooseObject("PhaseFieldApp", GrandPotentialInterface);

InputParameters
GrandPotentialInterface::validParams()
{
  InputParameters params = Material::validParams();
  params += MoelansInterfaceFits::validParams();
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
  MooseEnum reference_sigma("max median", "max");
  params.addParam<MooseEnum>(
      "reference_sigma",
      reference_sigma,
      "Interface that is assigned gamma = 1.5 and the interfacial width 'width': the one with the "
      "largest (max) or the median (median) interfacial free energy");
  params.addParam<unsigned int>("sigma_index",
                                "Sigma index to choose gamma = 1.5 for. Omit this to choose it "
                                "according to reference_sigma.");
  params.addParamNamesToGroup("mu_name reference_sigma sigma_index", "Advanced");
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

  // determine the reference interfacial free energy (or use explicit user choice). With the
  // largest one as reference all other interfaces have gamma < 1.5 and are wider than 'width'.
  unsigned int reference;
  if (isParamValid("sigma_index"))
    reference = getParam<unsigned int>("sigma_index");
  else if (getParam<MooseEnum>("reference_sigma") == "max")
    reference = std::max_element(_sigma.begin(), _sigma.end()) - _sigma.begin();
  else
  {
    std::vector<size_t> indices;
    Moose::indirectSort(_sigma.begin(), _sigma.end(), indices);
    reference = indices[(indices.size() - 1) / 2];
  }

  // set the reference gamma to 1.5 and use analytical expression for kappa and mu (m)
  _gamma[reference] = 1.5;
  _kappa = 3.0 / 4.0 * _sigma[reference] * _width;
  _mu = 6.0 * _sigma[reference] / _width;

  const auto fit = getParam<MooseEnum>("interface_fit").getEnum<MoelansInterfaceFits::Fit>();

  // set all other gammas
  for (unsigned int i = 0; i < _n_pair; ++i)
  {
    // skip the already calculated reference value
    if (i == reference)
      continue;

    const Real g = _sigma[i] / std::sqrt(_mu * _kappa);
    if (!MoelansInterfaceFits::inRange(g * g, fit))
      paramError("sigma",
                 "The interfacial free energy ",
                 _sigma[i],
                 " gives g = ",
                 g,
                 ", which is outside the range 0.098 <= g <= 0.766 (0.53 <= gamma <= 40) covered "
                 "by interface_fit = moelans2022.");

    // estimate for gamma from polynomial expansion
    _gamma[i] = 1.0 / MoelansInterfaceFits::inverseGamma(g * g, fit);
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
