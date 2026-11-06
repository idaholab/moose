//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicalSolidPropertiesMaterial.h"
#include "MechanicalSolidProperties.h"

registerMooseObject("SolidPropertiesApp", MechanicalSolidPropertiesMaterial);
registerMooseObject("SolidPropertiesApp", ADMechanicalSolidPropertiesMaterial);

template <bool is_ad>
InputParameters
MechanicalSolidPropertiesMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("temperature", "Temperature");
  params.addRequiredParam<UserObjectName>("sp", "The name of the user object for solid properties");
  params.addParam<std::string>("E", "E", "Name to be used for Young's modulus");
  params.addParam<std::string>("nu", "nu", "Name to be used for Poisson's ratio");
  params.addParam<std::string>(
      "alpha", "alpha", "Name to be used for the coefficient of thermal expansion");
  params.addClassDescription("Computes solid mechanical properties as a function of temperature");
  return params;
}

template <bool is_ad>
MechanicalSolidPropertiesMaterialTempl<is_ad>::MechanicalSolidPropertiesMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _temperature(coupledGenericValue<is_ad>("temperature")),

    _E(declareGenericProperty<Real, is_ad>(getParam<std::string>("E"))),
    _nu(declareGenericProperty<Real, is_ad>(getParam<std::string>("nu"))),
    _alpha(declareGenericProperty<Real, is_ad>(getParam<std::string>("alpha"))),

    _sp(getUserObject<MechanicalSolidProperties>("sp"))
{
}

template <bool is_ad>
void
MechanicalSolidPropertiesMaterialTempl<is_ad>::computeQpProperties()
{
  _E[_qp] = _sp.E_from_T(_temperature[_qp]);
  _nu[_qp] = _sp.nu_from_T(_temperature[_qp]);
  _alpha[_qp] = _sp.alpha_from_T(_temperature[_qp]);
}

template class MechanicalSolidPropertiesMaterialTempl<false>;
template class MechanicalSolidPropertiesMaterialTempl<true>;
