//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstant2DArray.h"

registerMooseObject("MooseApp", GenericConstant2DArray);
registerMooseObject("MooseApp", ADGenericConstant2DArray);

template <bool is_ad>
InputParameters
GenericConstant2DArrayTempl<is_ad>::validParams()
{

  InputParameters params = Material::validParams();
  params.addRequiredParam<std::string>("prop_name",
                                       "The names of the properties this material will have");
  params.addRequiredParam<RealEigenMatrix>("prop_value",
                                           "The values associated with the named properties");
  params.declareControllable("prop_value");
  params.addClassDescription(
      "A material evaluating one material property in type of RealEigenMatrix");
  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";
  return params;
}

template <bool is_ad>
GenericConstant2DArrayTempl<is_ad>::GenericConstant2DArrayTempl(const InputParameters & parameters)
  : Material(parameters),
    _prop_name(getParam<std::string>("prop_name")),
    _prop_value(getParam<RealEigenMatrix>("prop_value")),
    _property(declareGenericProperty<RealEigenMatrix, is_ad>(_prop_name))
{
}

template <bool is_ad>
void
GenericConstant2DArrayTempl<is_ad>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <bool is_ad>
void
GenericConstant2DArrayTempl<is_ad>::computeQpProperties()
{
  auto & qp_prop = _property[_qp];
  const auto m = _prop_value.rows(), n = _prop_value.cols();
  qp_prop.resize(m, n);
  for (const auto i : make_range(m))
    for (const auto j : make_range(n))
      qp_prop(i, j) = _prop_value(i, j);
}

template class GenericConstant2DArrayTempl<false>;
template class GenericConstant2DArrayTempl<true>;
