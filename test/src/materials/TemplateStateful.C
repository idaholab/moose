//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TemplateStateful.h"

registerMooseObject("MooseTestApp", TemplateStateful);
registerMooseObject("MooseTestApp", ADTemplateStateful);

template <bool is_ad>
InputParameters
TemplateStatefulTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>("property_name",
                                                "Name of porosity material property");

  return params;
}

template <bool is_ad>
TemplateStatefulTempl<is_ad>::TemplateStatefulTempl(const InputParameters & parameters)
  : Material(parameters),
    _property(declareGenericProperty<Real, is_ad>("property_name")),
    _property_old(getMaterialPropertyOld<Real>("property_name"))
{
}

template <bool is_ad>
void
TemplateStatefulTempl<is_ad>::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

template <bool is_ad>
void
TemplateStatefulTempl<is_ad>::computeQpProperties()
{
  _property[_qp] = 0.0;
}
