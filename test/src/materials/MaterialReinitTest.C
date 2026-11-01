//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "MaterialReinitTest.h"

registerMooseObject("MooseTestApp", MaterialReinitTest);
registerMooseObject("MooseTestApp", ADMaterialReinitTest);

template <bool is_ad>
InputParameters
MaterialReinitTestTempl<is_ad>::validParams()
{
  auto params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("property",
                                                "The property supplied by this material.");
  params.addParam<Real>("value", 0.0, "A constant contribution to the supplied property.");
  params.addParam<MaterialPropertyName>(
      "coupled_property", "An optional material property to add to the supplied property.");
  params.addParam<bool>("or_zero_coupled_property",
                        false,
                        "Whether to retrieve 'coupled_property' with "
                        "getGenericOrZeroMaterialProperty, so it is zero if no material "
                        "declares it.");
  params.addParam<bool>(
      "error_on_face", false, "Whether to error if the face copy of this material is computed.");
  params.addParam<bool>(
      "error_on_boundary", false, "Whether to error if this material is computed on a boundary.");
  params.addParam<bool>("error_on_neighbor",
                        false,
                        "Whether to error if the neighbor copy of this material is computed.");
  params.addParam<bool>(
      "error_on_volume", false, "Whether to error if the block copy of this material is computed.");
  return params;
}

template <bool is_ad>
MaterialReinitTestTempl<is_ad>::MaterialReinitTestTempl(const InputParameters & parameters)
  : Material(parameters),
    _property(declareGenericProperty<Real, is_ad>("property")),
    _value(getParam<Real>("value")),
    _coupled_property(!isParamValid("coupled_property") ? nullptr
                      : getParam<bool>("or_zero_coupled_property")
                          ? &getGenericOrZeroMaterialProperty<Real, is_ad>("coupled_property")
                          : &getGenericMaterialProperty<Real, is_ad>("coupled_property")),
    _error_on_face(getParam<bool>("error_on_face")),
    _error_on_boundary(getParam<bool>("error_on_boundary")),
    _error_on_neighbor(getParam<bool>("error_on_neighbor")),
    _error_on_volume(getParam<bool>("error_on_volume"))
{
}

template <bool is_ad>
void
MaterialReinitTestTempl<is_ad>::computeQpProperties()
{
  if (_error_on_volume && materialDataType() == Moose::BLOCK_MATERIAL_DATA)
    mooseError("The block copy of ", name(), " should not have been computed.");

  if (_error_on_face && materialDataType() == Moose::FACE_MATERIAL_DATA)
    mooseError("The face copy of ", name(), " should not have been computed.");

  if (_error_on_boundary && materialDataType() == Moose::BOUNDARY_MATERIAL_DATA)
    mooseError(name(), " should not have been computed on the boundary.");

  if (_error_on_neighbor && materialDataType() == Moose::NEIGHBOR_MATERIAL_DATA)
    mooseError("The neighbor copy of ", name(), " should not have been computed.");

  _property[_qp] = _value;
  if (_coupled_property)
    _property[_qp] += (*_coupled_property)[_qp];
}

template class MaterialReinitTestTempl<false>;
template class MaterialReinitTestTempl<true>;
