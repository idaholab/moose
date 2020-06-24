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

InputParameters
VectorCoupledValuesMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("variable", "Coupled variable");
  params.addParam<bool>(
      "request_dotdot", true, "Whether we should request second time derivative quantities");
  return params;
}

VectorCoupledValuesMaterial::VectorCoupledValuesMaterial(const InputParameters & parameters)
  : Material(parameters),
    _request_dotdot(getParam<bool>("request_dotdot")),
    _value(coupledVectorValue("variable")),
    _ad_value(adCoupledVectorValue("variable")),
    _dot(coupledVectorDot("variable")),
    _dot_dot(_request_dotdot ? &coupledVectorDotDot("variable") : nullptr),
    _dot_du(coupledVectorDotDu("variable")),
    _dot_dot_du(_request_dotdot ? &coupledVectorDotDotDu("variable") : nullptr),

    _var_name(getVectorVar("variable", 0)->name()),
    _value_prop(declareProperty<RealVectorValue>(_var_name + "_value")),
    _ad_value_prop(declareADProperty<RealVectorValue>(_var_name + "_ad_value")),
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
  _ad_value_prop[_qp] = _ad_value[_qp];
  _dot_prop[_qp] = _dot[_qp];
  _dot_du_prop[_qp] = _dot_du[_qp];
  if (_request_dotdot)
  {
    _dot_dot_prop[_qp] = (*_dot_dot)[_qp];
    _dot_dot_du_prop[_qp] = (*_dot_dot_du)[_qp];
  }
}
