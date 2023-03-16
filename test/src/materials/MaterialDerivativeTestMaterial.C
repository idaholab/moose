//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialDerivativeTestMaterial.h"

registerMooseObject("MooseTestApp", MaterialDerivativeTestMaterial);

InputParameters
MaterialDerivativeTestMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("var1", "First nonlinear variable");
  params.addRequiredCoupledVar("var2", "Second nonlinear variable");
  return params;
}

MaterialDerivativeTestMaterial::MaterialDerivativeTestMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _p(declareProperty<Real>("material_derivative_test_property")),
    _dpdu(
        declarePropertyDerivative<Real>("material_derivative_test_property", coupledName("var1"))),
    _dpdv(
        declarePropertyDerivative<Real>("material_derivative_test_property", coupledName("var2"))),
    _u(coupledValue("var1")),
    _v(coupledValue("var2"))
{
}

void
MaterialDerivativeTestMaterial::computeQpProperties()
{
  // p(u,v) = u^2 * v
  _p[_qp] = _u[_qp] * _u[_qp] * _v[_qp];
  _dpdu[_qp] = 2 * _u[_qp] * _v[_qp];
  _dpdv[_qp] = _u[_qp] * _u[_qp];
}
