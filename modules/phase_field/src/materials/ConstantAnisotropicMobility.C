//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantAnisotropicMobility.h"

registerMooseObject("PhaseFieldApp", ConstantAnisotropicMobility);
registerMooseObject("PhaseFieldApp", ADConstantAnisotropicMobility);

template <bool is_ad>
InputParameters
ConstantAnisotropicMobilityTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Provide a constant mobility tensor value");
  params.addRequiredParam<MaterialPropertyName>("M_name",
                                                "Name of the mobility tensor property to generate");
  params.addRequiredRangeCheckedParam<std::vector<Real>>(
      "tensor", "tensor_size=9", "Tensor values");
  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";
  return params;
}

template <bool is_ad>
ConstantAnisotropicMobilityTempl<is_ad>::ConstantAnisotropicMobilityTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _mobility_values(getParam<std::vector<Real>>("tensor")),
    _mobility_name(getParam<MaterialPropertyName>("M_name")),
    _mobility(declareGenericProperty<RealTensorValue, is_ad>(_mobility_name))
{
}

template <bool is_ad>
void
ConstantAnisotropicMobilityTempl<is_ad>::computeQpProperties()
{
  for (const auto a : make_range(Moose::dim))
    for (const auto b : make_range(Moose::dim))
      _mobility[_qp](a, b) = _mobility_values[a * 3 + b];
}

template class ConstantAnisotropicMobilityTempl<false>;
template class ConstantAnisotropicMobilityTempl<true>;
