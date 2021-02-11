//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledGradientMaterial.h"

registerMooseObject("MooseTestApp", ADCoupledGradientMaterial);

InputParameters
ADCoupledGradientMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("mat_prop",
                                                "Name of the property this material defines that "
                                                "will be equal to the coupled variable value");
  params.addRequiredParam<MaterialPropertyName>(
      "grad_mat_prop",
      "Name of the material property gradient that will be equal to the coupled variable gradient");
  params.addRequiredCoupledVar("u", "The coupled variable");
  return params;
}

ADCoupledGradientMaterial::ADCoupledGradientMaterial(const InputParameters & parameters)
  : Material(parameters),
    _mat_prop_name(getParam<MaterialPropertyName>("mat_prop")),
    _grad_mat_prop_name(getParam<MaterialPropertyName>("grad_mat_prop")),
    _mat_prop(declareADProperty<Real>(_mat_prop_name)),
    _grad_mat_prop(declareADProperty<RealVectorValue>(_grad_mat_prop_name)),
    _u(adCoupledValue("u")),
    _grad_u(adCoupledGradient("u"))
{
}

void
ADCoupledGradientMaterial::computeQpProperties()
{
  _mat_prop[_qp] = _u[_qp];
  _grad_mat_prop[_qp] = _grad_u[_qp];
}
