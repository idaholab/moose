//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VarCouplingMaterialEigen.h"

registerMooseObject("MooseTestApp", VarCouplingMaterialEigen);

InputParameters
VarCouplingMaterialEigen::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("var", "The variable to be coupled in");
  params.addRequiredParam<std::string>("material_prop_name", "Property name");
  return params;
}

VarCouplingMaterialEigen::VarCouplingMaterialEigen(const InputParameters & parameters)
  : Material(parameters),
    _var(coupledValue("var")),
    _var_old(coupledValueOld("var")),
    _propname(getParam<std::string>("material_prop_name")),
    _mat(declareProperty<Real>(_propname)),
    _mat_old(declareProperty<Real>(_propname + "_old"))
{
}

void
VarCouplingMaterialEigen::computeQpProperties()
{
  _mat[_qp] = _var[_qp];
  _mat_old[_qp] = _var_old[_qp];
}
