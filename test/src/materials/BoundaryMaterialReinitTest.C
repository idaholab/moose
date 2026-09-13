//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "BoundaryMaterialReinitTest.h"

registerMooseObject("MooseTestApp", BoundaryMaterialReinitTest);

InputParameters
BoundaryMaterialReinitTest::validParams()
{
  auto params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("property",
                                                "The property supplied by this material.");
  params.addParam<Real>("value", 0.0, "A constant contribution to the supplied property.");
  params.addParam<MaterialPropertyName>(
      "coupled_property", "An optional material property to add to the supplied property.");
  params.addParam<bool>(
      "error_on_face", false, "Whether to error if the face copy of this material is computed.");
  return params;
}

BoundaryMaterialReinitTest::BoundaryMaterialReinitTest(const InputParameters & parameters)
  : Material(parameters),
    _property(declareProperty<Real>("property")),
    _value(getParam<Real>("value")),
    _coupled_property(isParamValid("coupled_property")
                          ? &getMaterialProperty<Real>("coupled_property")
                          : nullptr),
    _error_on_face(getParam<bool>("error_on_face"))
{
}

void
BoundaryMaterialReinitTest::computeQpProperties()
{
  if (_error_on_face && materialDataType() == Moose::FACE_MATERIAL_DATA)
    mooseError("The face copy of ", name(), " should not have been computed.");

  _property[_qp] = _value;
  if (_coupled_property)
    _property[_qp] += (*_coupled_property)[_qp];
}
