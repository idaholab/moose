//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrheniusMaterialProperty.h"

#include "PhysicalConstants.h"

#include "libmesh/utility.h"

registerMooseObject("MiscApp", ArrheniusMaterialProperty);
registerMooseObject("MiscApp", ADArrheniusMaterialProperty);

template <bool is_ad>
InputParameters
ArrheniusMaterialPropertyTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();

  params.addClassDescription(
      "Arbitrary material property of the sum of an arbitary number ($i$) of "
      "Arrhenius functions $A_i * \\exp{-Q_i / (RT)}$, where $A_i$ is the frequency "
      "factor, $Q_i$ is the activation energy, and $R$ is the gas constant.");

  params.addRequiredParam<std::string>("property_name",
                                       "Specify the name of this material property");
  params.addRequiredCoupledVar("temperature", "Coupled temperature");
  params.addRequiredParam<std::vector<Real>>("frequency_factor",
                                             "List of Arrhenius pre-exponential coefficients");
  params.addRequiredParam<std::vector<Real>>("activation_energy", "List of activation energies");
  params.addRangeCheckedParam<Real>("gas_constant",
                                    PhysicalConstants::ideal_gas_constant,
                                    "gas_constant>0",
                                    "Gas constant for Arrhenius function");
  params.addRangeCheckedParam<Real>(
      "initial_temperature",
      1.0,
      "initial_temperature > 0",
      "Initial temperature utilized for initialization of stateful properties");

  return params;
}

template <bool is_ad>
ArrheniusMaterialPropertyTempl<is_ad>::ArrheniusMaterialPropertyTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _diffusivity(
        this->template declareGenericProperty<Real, is_ad>(getParam<std::string>("property_name"))),
    _diffusivity_dT(this->template declareGenericProperty<Real, is_ad>(
        getParam<std::string>("property_name") + "_dT")),
    _temperature(this->template coupledGenericValue<is_ad>("temperature")),
    _D_0(getParam<std::vector<Real>>("frequency_factor")),
    _Q(getParam<std::vector<Real>>("activation_energy")),
    _R(getParam<Real>("gas_constant")),
    _number(_D_0.size()),
    _initial_temperature(this->template getParam<Real>("initial_temperature"))
{
  if (_number != _Q.size())
    paramError("frequency_factor",
               "frequency_factor and activation_energy must have the same number of entries");
  if (_number == 0)
    paramError("frequency_factor",
               "At least one frequency_factor and activation_energy parameter must be given");
}

template <bool is_ad>
void
ArrheniusMaterialPropertyTempl<is_ad>::initQpStatefulProperties()
{
  _diffusivity[_qp] = 0.0;
  _diffusivity_dT[_qp] = 0.0;

  for (unsigned int i = 0; i < _number; ++i)
  {
    _diffusivity[_qp] += _D_0[i] * std::exp(-_Q[i] / _R / _initial_temperature);
    _diffusivity_dT[_qp] -= _D_0[i] * std::exp(-_Q[i] / _R / _initial_temperature) * _Q[i];
  }

  _diffusivity_dT[_qp] /= _R * Utility::pow<2>(_initial_temperature);
}

template <bool is_ad>
void
ArrheniusMaterialPropertyTempl<is_ad>::computeQpProperties()
{
  const GenericReal<is_ad> temp = std::max(_temperature[_qp], 1e-30);

  _diffusivity[_qp] = 0.0;
  _diffusivity_dT[_qp] = 0.0;

  for (unsigned int i = 0; i < _number; ++i)
  {
    _diffusivity[_qp] += _D_0[i] * std::exp(-_Q[i] / _R / temp);
    _diffusivity_dT[_qp] += _D_0[i] * std::exp(-_Q[i] / _R / temp) * _Q[i];
  }

  _diffusivity_dT[_qp] /= (_R * Utility::pow<2>(temp));
}
