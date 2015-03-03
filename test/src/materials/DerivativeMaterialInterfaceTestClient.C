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
#include "DerivativeMaterialInterfaceTestClient.h"

template<>
InputParameters validParams<DerivativeMaterialInterfaceTestClient>()
{
  InputParameters params = validParams<Material>();
  return params;
}

DerivativeMaterialInterfaceTestClient::DerivativeMaterialInterfaceTestClient(const std::string & name,
                                                                                 InputParameters parameters) :
    DerivativeMaterialInterface<Material>(name, parameters),
    _prop0(getMaterialPropertyDerivative<Real>("prop","c")), // fetch non-existing derivative
    _prop1(getMaterialPropertyDerivative<Real>("prop","a")),
    _prop2(getMaterialPropertyDerivative<Real>("prop","b")),
    _prop3(getMaterialPropertyDerivative<Real>("prop","a", "b")), // fetch alphabetically sorted (but declared unsorted)
    _prop4(getMaterialPropertyDerivative<Real>("prop","a", "c")),
    _prop5(getMaterialPropertyDerivative<Real>("prop","c", "b", "a")) // fetch unsorted (declared unsorted, but differently unsorted)
{
}


void
DerivativeMaterialInterfaceTestClient::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    if (_prop0[_qp] != 0.0 || _prop1[_qp] != 1.0 || _prop2[_qp] != 2.0 || _prop3[_qp] != 3.0 || _prop4[_qp] != 4.0 || _prop5[_qp] != 5.0)
      mooseError("Unexpected DerivativeMaterial property value.");
  }
}
