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

InputParameters
CoupledValuesMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("variable", "Coupled variable");
  return params;
}

CoupledValuesMaterial::CoupledValuesMaterial(const InputParameters & parameters)
  : Material(parameters),
    _value(coupledValue("variable")),
    _dot(coupledDot("variable")),
    _dot_dot(coupledDotDot("variable")),
    _dot_du(coupledDotDu("variable")),
    _dot_dot_du(coupledDotDotDu("variable")),

    _var_name(getVar("variable", 0)->name()),
    _value_prop(declareProperty<Real>(_var_name + "_value")),
    _dot_prop(declareProperty<Real>(_var_name + "_dot")),
    _dot_dot_prop(declareProperty<Real>(_var_name + "_dot_dot")),
    _dot_du_prop(declareProperty<Real>(_var_name + "_dot_du")),
    _dot_dot_du_prop(declareProperty<Real>(_var_name + "_dot_dot_du"))
{
}

void
CoupledValuesMaterial::computeQpProperties()
{
  _value_prop[_qp] = _value[_qp];
  _dot_prop[_qp] = _dot[_qp];
  _dot_dot_prop[_qp] = _dot_dot[_qp];
  _dot_du_prop[_qp] = _dot_du[_qp];
  _dot_dot_du_prop[_qp] = _dot_dot_du[_qp];
}
