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
#include "DerivativeMaterialInterfaceTestProvider.h"

// libmesh includes
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
