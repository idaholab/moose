/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "VariableGradientMaterial.h"

template <>
InputParameters
validParams<VariableGradientMaterial>()
{
  InputParameters params = validParams<Material>();
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
