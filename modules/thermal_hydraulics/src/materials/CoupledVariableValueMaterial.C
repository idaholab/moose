//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledVariableValueMaterial.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("ThermalHydraulicsApp", CoupledVariableValueMaterial);
registerMooseObject("ThermalHydraulicsApp", ADCoupledVariableValueMaterial);

template <bool is_ad>
InputParameters
CoupledVariableValueMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "The name of the material property where we store the variable values.");
  params.addRequiredCoupledVar(
      "coupled_variable", "The coupled variable that will be stored into the material property");
  params.addClassDescription("Stores values of a variable into material properties");
  return params;
}

template <bool is_ad>
CoupledVariableValueMaterialTempl<is_ad>::CoupledVariableValueMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _prop_name(getParam<MaterialPropertyName>("prop_name")),
    _prop(declareGenericProperty<Real, is_ad>(_prop_name)),
    _value((!is_ad) ? coupledValue("coupled_variable") : _zero),
    _ad_value((is_ad) ? adCoupledValue("coupled_variable") : _ad_zero)
{
}

template <bool is_ad>
void
CoupledVariableValueMaterialTempl<is_ad>::computeQpProperties()
{
  if (is_ad)
    _prop[_qp] = MetaPhysicL::raw_value(_ad_value[_qp]);
  else
    _prop[_qp] = _value[_qp];
}

template class CoupledVariableValueMaterialTempl<false>;
template class CoupledVariableValueMaterialTempl<true>;
