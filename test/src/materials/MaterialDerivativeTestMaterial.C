/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "MaterialDerivativeTestMaterial.h"

template <>
InputParameters
validParams<MaterialDerivativeTestMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("var1", "First nonlinear variable");
  params.addRequiredCoupledVar("var2", "Second nonlinear variable");
  return params;
}

MaterialDerivativeTestMaterial::MaterialDerivativeTestMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _p(declareProperty<Real>("material_derivative_test_property")),
    _dpdu(declarePropertyDerivative<Real>("material_derivative_test_property",
                                          getVar("var1", 0)->name())),
    _dpdv(declarePropertyDerivative<Real>("material_derivative_test_property",
                                          getVar("var2", 0)->name())),
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
