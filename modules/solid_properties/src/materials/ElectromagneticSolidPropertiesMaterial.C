//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElectromagneticSolidPropertiesMaterial.h"
#include "ElectromagneticSolidProperties.h"

registerMooseObject("SolidPropertiesApp", ElectromagneticSolidPropertiesMaterial);
registerMooseObject("SolidPropertiesApp", ADElectromagneticSolidPropertiesMaterial);

template <bool is_ad>
InputParameters
ElectromagneticSolidPropertiesMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("temperature", "Temperature");
  params.addRequiredParam<UserObjectName>("sp", "The name of the user object for solid properties");
  params.addParam<std::string>("electrical_resistivity_name",
                               "electrical_resistivity",
                               "Name to be used for the electrical resistivity material property");
  params.addParam<std::string>("electrical_conductivity_name",
                               "electrical_conductivity",
                               "Name to be used for the electrical conductivity material property");
  params.addParam<std::string>("magnetic_permeability_name",
                               "magnetic_permeability",
                               "Name to be used for the magnetic permeability material property");
  params.addClassDescription(
      "Computes solid electromagnetic properties as a function of temperature");
  return params;
}

template <bool is_ad>
ElectromagneticSolidPropertiesMaterialTempl<is_ad>::ElectromagneticSolidPropertiesMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _temperature(coupledGenericValue<is_ad>("temperature")),

    _electrical_resistivity(
        declareGenericProperty<Real, is_ad>(getParam<std::string>("electrical_resistivity_name"))),
    _electrical_conductivity(
        declareGenericProperty<Real, is_ad>(getParam<std::string>("electrical_conductivity_name"))),
    _magnetic_permeability(
        declareGenericProperty<Real, is_ad>(getParam<std::string>("magnetic_permeability_name"))),

    _sp(getUserObject<ElectromagneticSolidProperties>("sp"))
{
}

template <bool is_ad>
void
ElectromagneticSolidPropertiesMaterialTempl<is_ad>::computeQpProperties()
{
  _electrical_resistivity[_qp] = _sp.electrical_resistivity_from_T(_temperature[_qp]);
  _electrical_conductivity[_qp] = _sp.electrical_conductivity_from_T(_temperature[_qp]);
  _magnetic_permeability[_qp] = _sp.magnetic_permeability_from_T(_temperature[_qp]);
}

template class ElectromagneticSolidPropertiesMaterialTempl<false>;
template class ElectromagneticSolidPropertiesMaterialTempl<true>;
