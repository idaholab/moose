//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayleighNumber.h"

registerMooseObject("NavierStokesApp", RayleighNumber);

InputParameters
RayleighNumber::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addClassDescription("Postprocessor that computes the Rayleigh number for free flow with "
                             "natural circulation");

  // Compute it from a density change
  params.addRequiredParam<PostprocessorName>("rho_ave", "Average density");
  params.addParam<PostprocessorName>("rho_min", "Minimum density");
  params.addParam<PostprocessorName>("rho_max", "Maximum density");

  // Compute it from a temperature change
  params.addParam<PostprocessorName>("beta",
                                     "Absolute value of fluid volumetric expansion coefficient");
  params.addParam<PostprocessorName>("T_hot", "Maximum temperature, or hot source temperature");
  params.addParam<PostprocessorName>("T_cold", "Minimum temperature, or cold source temperature");

  params.addRequiredParam<PostprocessorName>("l", "Characteristic distance");
  params.addRequiredParam<PostprocessorName>("mu_ave", "Average value of the dynamic viscosity");
  params.addRequiredParam<PostprocessorName>("k_ave", "Average value of the thermal conductivity");
  params.addRequiredParam<PostprocessorName>("cp_ave",
                                             "Average value of the specific thermal capacity");
  params.addRequiredParam<Real>("gravity_magnitude",
                                "Gravity vector magnitude in the direction of interest");

  return params;
}

RayleighNumber::RayleighNumber(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _rho_min(isParamValid("rho_min") ? &getPostprocessorValue("rho_min") : nullptr),
    _rho_max(isParamValid("rho_max") ? &getPostprocessorValue("rho_max") : nullptr),
    _rho_ave(getPostprocessorValue("rho_ave")),
    _beta(isParamValid("beta") ? &getPostprocessorValue("beta") : nullptr),
    _T_hot(isParamValid("T_hot") ? &getPostprocessorValue("T_hot") : nullptr),
    _T_cold(isParamValid("T_cold") ? &getPostprocessorValue("T_cold") : nullptr),
    _l(getPostprocessorValue("l")),
    _mu(getPostprocessorValue("mu_ave")),
    _k(getPostprocessorValue("k_ave")),
    _cp(getPostprocessorValue("cp_ave")),
    _gravity(getParam<Real>("gravity_magnitude"))
{
  // Check that enough information has been given
  if ((!_rho_ave || !_rho_min || !_rho_max) && (!_rho_ave || !_beta || !_T_hot || !_T_cold))
    mooseError("To compute the density difference for the Rayleigh number, the density "
               "min/max/average or the expansion coefficient and the temperature min/max "
               "must be provided.");
}

Real
RayleighNumber::getValue()
{
  Real drho;
  if (_rho_min)
    drho = *_rho_max - *_rho_min;
  else
    drho = *_beta * (*_T_hot - *_T_cold) * _rho_ave;

  if (_mu <= 0 || _k <= 0 || _rho_ave <= 0)
    mooseError("Average viscosity, density and thermal conductivity should be strictly positive");

  return drho * std::pow(_l, 3) * _gravity * _cp * _rho_ave / (_mu * _k);
}
