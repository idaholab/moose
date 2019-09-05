//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorCoupledValuesMaterial.h"

registerMooseObject("MooseTestApp", VectorCoupledValuesMaterial);

template <>
InputParameters
validParams<VectorCoupledValuesMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("variable", "Coupled variable");
  return params;
}

VectorCoupledValuesMaterial::VectorCoupledValuesMaterial(const InputParameters & parameters)
  : Material(parameters),
    _value(coupledVectorValue("variable")),
    _dot(coupledVectorDot("variable")),
    _dot_dot(coupledVectorDotDot("variable")),
    _dot_du(coupledVectorDotDu("variable")),
    _dot_dot_du(coupledVectorDotDotDu("variable")),

    _var_name(getVectorVar("variable", 0)->name()),
    _value_prop(declareProperty<RealVectorValue>(_var_name + "_value")),
    _dot_prop(declareProperty<RealVectorValue>(_var_name + "_dot")),
    _dot_dot_prop(declareProperty<RealVectorValue>(_var_name + "_dot_dot")),
    _dot_du_prop(declareProperty<Real>(_var_name + "_dot_du")),
    _dot_dot_du_prop(declareProperty<Real>(_var_name + "_dot_dot_du"))
{
}

void
VectorCoupledValuesMaterial::computeQpProperties()
{
  _value_prop[_qp] = _value[_qp];
  _dot_prop[_qp] = _dot[_qp];
  _dot_dot_prop[_qp] = _dot_dot[_qp];
  _dot_du_prop[_qp] = _dot_du[_qp];
  _dot_dot_du_prop[_qp] = _dot_dot_du[_qp];
}
