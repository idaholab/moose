//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarPropFromFunctorProp.h"

registerMooseObject("MooseTestApp", ScalarPropFromFunctorProp);

InputParameters
ScalarPropFromFunctorProp::validParams()
{
  auto params = Material::validParams();
  params.addRequiredParam<MooseFunctorName>("functor", "A functor");
  params.addRequiredParam<MaterialPropertyName>("prop", "The property to declare");
  return params;
}

ScalarPropFromFunctorProp::ScalarPropFromFunctorProp(const InputParameters & params)
  : Material(params),
    _functor(getFunctor<ADReal>("functor")),
    _prop(declareADProperty<Real>("prop"))
{
}

void
ScalarPropFromFunctorProp::computeQpProperties()
{
  if (!_bnd && !_neighbor)
    _prop[_qp] = _functor(std::make_tuple(_current_elem, _qp, _qrule), Moose::currentState());
  else
    _prop[_qp] =
        _functor(std::make_tuple(_current_elem, _current_side, _qp, _qrule), Moose::currentState());
}
