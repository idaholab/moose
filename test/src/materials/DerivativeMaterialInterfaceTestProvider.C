//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DerivativeMaterialInterfaceTestProvider.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<DerivativeMaterialInterfaceTestProvider>()
{
  InputParameters params = validParams<Material>();
  return params;
}

DerivativeMaterialInterfaceTestProvider::DerivativeMaterialInterfaceTestProvider(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _prop1(declarePropertyDerivative<Real>("prop", "a")),
    _prop2(declarePropertyDerivative<Real>("prop", "b")),
    _prop3(declarePropertyDerivative<Real>("prop", "b", "a")),
    _prop4(declarePropertyDerivative<Real>("prop", "a", "c")),
    _prop5(declarePropertyDerivative<Real>("prop", "b", "c", "a")),
    _prop6(declareProperty<dof_id_type>("elementid"))
{
}

void
DerivativeMaterialInterfaceTestProvider::computeQpProperties()
{
  _prop1[_qp] = 1.0;
  _prop2[_qp] = 2.0;
  _prop3[_qp] = 3.0;
  _prop4[_qp] = 4.0;
  _prop5[_qp] = 5.0;

  _prop6[_qp] = _current_elem->id();
}
