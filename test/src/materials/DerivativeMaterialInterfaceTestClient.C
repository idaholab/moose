//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DerivativeMaterialInterfaceTestClient.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseTestApp", DerivativeMaterialInterfaceTestClient);
registerMooseObject("MooseTestApp", ADDerivativeMaterialInterfaceTestClient);

template <bool is_ad>
InputParameters
DerivativeMaterialInterfaceTestClientTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<MaterialPropertyName>("prop_name", "", "Name of the property to be retrieved");
  return params;
}

template <bool is_ad>
DerivativeMaterialInterfaceTestClientTempl<is_ad>::DerivativeMaterialInterfaceTestClientTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _prop_name(getParam<MaterialPropertyName>("prop_name")),
    _by_name(_prop_name == ""),
    _prop0(_by_name
               ? getMaterialPropertyDerivativeByName<Real, is_ad>("prop", "c")
               : getMaterialPropertyDerivative<Real, is_ad>("prop_name",
                                                            "c")), // fetch non-existing derivative
    _prop1(_by_name ? getMaterialPropertyDerivativeByName<Real, is_ad>("prop", "a")
                    : getMaterialPropertyDerivative<Real, is_ad>("prop_name", "a")),
    _prop2(_by_name ? getMaterialPropertyDerivativeByName<Real, is_ad>("prop", "b")
                    : getMaterialPropertyDerivative<Real, is_ad>("prop_name", "b")),
    _prop3(_by_name
               ? getMaterialPropertyDerivativeByName<Real, is_ad>("prop", "a", "b")
               : getMaterialPropertyDerivative<Real, is_ad>(
                     "prop_name", "a", "b")), // fetch alphabetically sorted (but declared unsorted)
    _prop4(_by_name ? getMaterialPropertyDerivativeByName<Real, is_ad>("prop", "a", "c")
                    : getMaterialPropertyDerivative<Real, is_ad>("prop_name", "a", "c")),
    _prop5(_by_name ? getMaterialPropertyDerivativeByName<Real, is_ad>("prop", "c", "b", "a")
                    : getMaterialPropertyDerivative<Real, is_ad>(
                          "prop_name",
                          "c",
                          "b",
                          "a")), // fetch unsorted (declared unsorted, but differently unsorted)
    _prop6(getDefaultMaterialProperty<dof_id_type>("elementid")), // check execution order
    _prop7(getDefaultMaterialProperty<Real, is_ad>("other_prop")),
    _prop8(getDefaultMaterialProperty<Real, is_ad>("invalid_prop"))
{
}

template <bool is_ad>
void
DerivativeMaterialInterfaceTestClientTempl<is_ad>::initialSetup()
{
  if (!_by_name)
    validateDerivativeMaterialPropertyBase<Real, is_ad>("prop_name");
}

template <bool is_ad>
void
DerivativeMaterialInterfaceTestClientTempl<is_ad>::computeQpProperties()
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

    if (_prop7[_qp] != 6.0)
      mooseError("Unexpected DerivativeMaterial property7 value. ", _prop7[_qp]);
    if (_prop8[_qp] != 0.0)
      mooseError("Unexpected DerivativeMaterial property8 value. ", _prop8[_qp]);
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

    if (_prop7[_qp] != 6.0)
      mooseError("Unexpected DerivativeMaterial property7 value. ", _prop7[_qp]);
    if (_prop8[_qp] != 0.0)
      mooseError("Unexpected DerivativeMaterial property8 value. ", _prop8[_qp]);
  }
  else
    mooseError("Unexpected DerivativeMaterial property name.");

  // check execution order (if the order is wrong the property is lagging by one element)
  if (_prop6[_qp] != _current_elem->id())
    mooseError("Wrong material execution order.");
}
