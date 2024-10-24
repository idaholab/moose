//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledValuesMaterial.h"

registerMooseObject("MooseTestApp", CoupledValuesMaterial);
registerMooseObject("MooseTestApp", ADCoupledValuesMaterial);

template <bool is_ad>
InputParameters
CoupledValuesMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("variable", "Coupled variable");
  return params;
}

template <bool is_ad>
CoupledValuesMaterialTempl<is_ad>::CoupledValuesMaterialTempl(const InputParameters & parameters)
  : Material(parameters),
    _value(coupledGenericValue<is_ad>("variable")),
    _dot(coupledGenericDot<is_ad>("variable")),
    _dot_dot(coupledGenericDotDot<is_ad>("variable")),
    // We dont have a generic override for this, and we don't need one yet
    _dot_du(coupledDotDu("variable")),
    _dot_dot_du(coupledDotDotDu("variable")),
    _var_name(getVar("variable", 0)->name()),
    _value_prop(declareGenericProperty<Real, is_ad>(_var_name + "_value")),
    _dot_prop(declareGenericProperty<Real, is_ad>(_var_name + "_dot")),
    _dot_dot_prop(declareGenericProperty<Real, is_ad>(_var_name + "_dot_dot")),
    _dot_du_prop(declareProperty<Real>(_var_name + "_dot_du")),
    _dot_dot_du_prop(declareProperty<Real>(_var_name + "_dot_dot_du"))
{
}

template <bool is_ad>
void
CoupledValuesMaterialTempl<is_ad>::computeQpProperties()
{
  _value_prop[_qp] = _value[_qp];
  _dot_prop[_qp] = _dot[_qp];
  _dot_dot_prop[_qp] = _dot_dot[_qp];
  _dot_du_prop[_qp] = _dot_du[_qp];
  _dot_dot_du_prop[_qp] = _dot_dot_du[_qp];
}

template class CoupledValuesMaterialTempl<false>;
template class CoupledValuesMaterialTempl<true>;
