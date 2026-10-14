//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADQuadraticMaterial.h"

registerKokkosADMaterial("MooseTestApp", KokkosADQuadraticMaterial);

InputParameters
KokkosADQuadraticMaterial::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addClassDescription("Test AD material computing k = k0 + a T^2 + g . x.");
  params.addRequiredCoupledVar("T", "The coupled variable");
  params.addParam<Real>("k0", 1, "The constant term");
  params.addParam<Real>("a", 1, "The coefficient of the quadratic term in T");
  params.addParam<RealVectorValue>(
      "g", RealVectorValue(0, 0, 0), "The coefficient vector of the position");
  params.addParam<MaterialPropertyName>("property_name", "k", "The name of the property");
  return params;
}

KokkosADQuadraticMaterial::KokkosADQuadraticMaterial(const InputParameters & parameters)
  : ADMaterial(parameters),
    _T(adCoupledValue("T")),
    _k0(getParam<Real>("k0")),
    _a(getParam<Real>("a")),
    _g(getParam<RealVectorValue>("g")),
    _k(declareKokkosProperty<Real>("property_name"))
{
}
