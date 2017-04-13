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

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<DerivativeMaterialInterfaceTestClient>()
{
  InputParameters params = validParams<Material>();
  params.addParam<MaterialPropertyName>("prop_name", "", "Name of the property to be retrieved");
  return params;
}

DerivativeMaterialInterfaceTestClient::DerivativeMaterialInterfaceTestClient(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _prop_name(getParam<MaterialPropertyName>("prop_name")),
    _by_name(_prop_name == ""),
    _prop0(_by_name ? getMaterialPropertyDerivativeByName<Real>("prop", "c")
                    : getMaterialPropertyDerivative<Real>("prop_name",
                                                          "c")), // fetch non-existing derivative
    _prop1(_by_name ? getMaterialPropertyDerivativeByName<Real>("prop", "a")
                    : getMaterialPropertyDerivative<Real>("prop_name", "a")),
    _prop2(_by_name ? getMaterialPropertyDerivativeByName<Real>("prop", "b")
                    : getMaterialPropertyDerivative<Real>("prop_name", "b")),
    _prop3(_by_name
               ? getMaterialPropertyDerivativeByName<Real>("prop", "a", "b")
               : getMaterialPropertyDerivative<Real>(
                     "prop_name", "a", "b")), // fetch alphabetically sorted (but declared unsorted)
    _prop4(_by_name ? getMaterialPropertyDerivativeByName<Real>("prop", "a", "c")
                    : getMaterialPropertyDerivative<Real>("prop_name", "a", "c")),
    _prop5(_by_name ? getMaterialPropertyDerivativeByName<Real>("prop", "c", "b", "a")
                    : getMaterialPropertyDerivative<Real>(
                          "prop_name",
                          "c",
                          "b",
                          "a")), // fetch unsorted (declared unsorted, but differently unsorted)
    _prop6(getDefaultMaterialProperty<dof_id_type>("elementid")) // check execution order
{
}

void
DerivativeMaterialInterfaceTestClient::initialSetup()
{
  if (!_by_name)
    validateDerivativeMaterialPropertyBase<Real>("prop_name");
}

void
DerivativeMaterialInterfaceTestClient::computeQpProperties()
{
  if (_by_name || _prop_name == "prop")
  {
    if (_prop0[_qp] != 0.0)
      mooseError("Uninitialized non-existing derivative (should be a zero property).");
    if (_prop1[_qp] == 0.0)
      mooseError(
          "property1 stayed at its zero default value. This indicates a broken execution order.");
    if (_prop1[_qp] != 1.0)
      mooseError("Unexpected DerivativeMaterial property1 value. ", _prop1[_qp]);
    if (_prop2[_qp] != 2.0)
      mooseError("Unexpected DerivativeMaterial property2 value. ", _prop2[_qp]);
    if (_prop3[_qp] != 3.0)
      mooseError("Unexpected DerivativeMaterial property3 value. ", _prop3[_qp]);
    if (_prop4[_qp] != 4.0)
      mooseError("Unexpected DerivativeMaterial property4 value. ", _prop4[_qp]);
    if (_prop5[_qp] != 5.0)
      mooseError("Unexpected DerivativeMaterial property5 value. ", _prop5[_qp]);
  }
  else if (_prop_name == "1.0")
  {
    if (_prop0[_qp] != 0.0)
      mooseError("Unexpected DerivativeMaterial default property0 value. ", _prop0[_qp]);
    if (_prop1[_qp] != 0.0)
      mooseError("Unexpected DerivativeMaterial default property1 value. ", _prop1[_qp]);
    if (_prop2[_qp] != 0.0)
      mooseError("Unexpected DerivativeMaterial default property2 value. ", _prop2[_qp]);
    if (_prop3[_qp] != 0.0)
      mooseError("Unexpected DerivativeMaterial default property3 value. ", _prop3[_qp]);
    if (_prop4[_qp] != 0.0)
      mooseError("Unexpected DerivativeMaterial default property4 value. ", _prop4[_qp]);
    if (_prop5[_qp] != 0.0)
      mooseError("Unexpected DerivativeMaterial default property5 value. ", _prop5[_qp]);
  }
  else
    mooseError("Unexpected DerivativeMaterial property name.");

  // check execution order (if the order is wrong the property is lagging by one element)
  if (_prop6[_qp] != _current_elem->id())
    mooseError("Wrong material execution order.");
}
