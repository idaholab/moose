//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableGradientMaterial.h"

registerMooseObject("PhaseFieldApp", VariableGradientMaterial);

InputParameters
VariableGradientMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute the norm of the gradient of a variable");
  params.addCoupledVar("variable", "Variable to compute the gradient magnitude of");
  params.addRequiredParam<MaterialPropertyName>(
      "prop", "Material property to store the gradient magnitude in");
  return params;
}

VariableGradientMaterial::VariableGradientMaterial(const InputParameters & parameters)
  : Material(parameters),
    _grad(coupledGradient("variable")),
    _prop(declareProperty<Real>(getParam<MaterialPropertyName>("prop")))
{
}

void
VariableGradientMaterial::computeQpProperties()
{
  _prop[_qp] = _grad[_qp].norm();
}
