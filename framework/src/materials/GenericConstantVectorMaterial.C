//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstantVectorMaterial.h"

registerMooseObject("MooseApp", GenericConstantVectorMaterial);
registerMooseObject("MooseApp", ADGenericConstantVectorMaterial);

template <bool is_ad>
InputParameters
GenericConstantVectorMaterialTempl<is_ad>::validParams()
{

  InputParameters params = Material::validParams();
  params.addRequiredParam<std::string>("prop_name",
                                       "The name of the property this material will have");
  params.addRequiredParam<RealVectorValue>("prop_value",
                                           "The values associated with the named property");
  params.declareControllable("prop_value");
  params.addClassDescription(
      "A material evaluating one material property in type of RealEigenVector");
  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";
  return params;
}

template <bool is_ad>
GenericConstantVectorMaterialTempl<is_ad>::GenericConstantVectorMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _prop_name(getParam<std::string>("prop_name")),
    _prop_value(getParam<RealVectorValue>("prop_value")),
    _property(declareProperty<RealVectorValue>(_prop_name))
{
}

template <bool is_ad>
void
GenericConstantVectorMaterialTempl<is_ad>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <bool is_ad>
void
GenericConstantVectorMaterialTempl<is_ad>::computeQpProperties()
{
  _property[_qp] = _prop_value;
}

template class GenericConstantVectorMaterialTempl<false>;
template class GenericConstantVectorMaterialTempl<true>;
