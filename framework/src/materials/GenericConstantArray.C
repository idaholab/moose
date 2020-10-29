//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstantArray.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", GenericConstantArray);
registerMooseObject("MooseApp", ADGenericConstantArray);

template <bool is_ad>
InputParameters
GenericConstantArrayTempl<is_ad>::validParams()
{

  InputParameters params = Material::validParams();
  params.addRequiredParam<std::string>("prop_name",
                                       "The names of the properties this material will have");
  params.addRequiredParam<RealEigenVector>("prop_value",
                                           "The values associated with the named properties");
  params.declareControllable("prop_value");
  params.addClassDescription(
      "A material evaluating one material property in type of RealEigenVector");
  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";
  return params;
}

template <bool is_ad>
GenericConstantArrayTempl<is_ad>::GenericConstantArrayTempl(const InputParameters & parameters)
  : Material(parameters),
    _prop_name(getParam<std::string>("prop_name")),
    _prop_value(getParam<RealEigenVector>("prop_value")),
    _property(declareGenericProperty<RealEigenVector, is_ad>(_prop_name))
{
}

template <bool is_ad>
void
GenericConstantArrayTempl<is_ad>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <bool is_ad>
void
GenericConstantArrayTempl<is_ad>::computeQpProperties()
{
  _property[_qp].resize(_prop_value.size());
  for (int i = 0; i < _prop_value.size(); i++)
    _property[_qp](i) = _prop_value(i);
}

template class GenericConstantArrayTempl<false>;
template class GenericConstantArrayTempl<true>;
