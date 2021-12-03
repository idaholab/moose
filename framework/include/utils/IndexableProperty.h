//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include <vector>

/**
 * IndexableProperty is a helper (proxy) object to obtain a scalar component
 * from a material property. Use it in objects that process a scalar quantity
 * instead of a `Real` material property to allow the user to supply any material
 * property of a type from the list below along with a component index parameter to
 * select a scalar component from the property value.
 */
template <typename T, bool is_ad>
class IndexableProperty
{
public:
  static InputParameters validParams();

  IndexableProperty(T * host,
                    const std::string & property_param = "property",
                    const std::string & component_param = "component");

  /// get the selected component value for the given quadrature point
  GenericReal<is_ad> operator[](int qp) const;

  /// integrity check
  void check() const;

protected:
  void checkComponents(unsigned int components) const;

  /// pointer to the host object
  T * _host;

  /// name of the input parameter containing the material property name
  const std::string & _property_param;
  /// name of the coupled material property (for error reporting)
  const std::string & _property_name;
  /// name of the input parameter containing the component index
  const std::string & _component_param;

  /// Index of the selected scalar component of the material property
  const std::vector<unsigned int> _component;

  ///@{ only one of those pointers will be non-null and pointing to the selected property
  const GenericOptionalMaterialProperty<Real, is_ad> & _property_real;
  const GenericOptionalMaterialProperty<std::vector<Real>, is_ad> & _property_std_vector;
  const GenericOptionalMaterialProperty<RealVectorValue, is_ad> & _property_real_vector_value;
  const GenericOptionalMaterialProperty<RankTwoTensor, is_ad> & _property_rank_two_tensor;
  const GenericOptionalMaterialProperty<RankThreeTensor, is_ad> & _property_rank_three_tensor;
  const GenericOptionalMaterialProperty<RankFourTensor, is_ad> & _property_rank_four_tensor;
  ///@}
};

template <typename T, bool is_ad>
InputParameters
IndexableProperty<T, is_ad>::validParams()
{
  auto params = T::validParams();
  params.template addRequiredParam<MaterialPropertyName>("property",
                                                         "The name of the material property");
  params.template addParam<std::vector<unsigned int>>(
      "component",
      "Index vector of the scalar component to extract from "
      "the material property (empty for scalar properties)");
  return params;
}

template <typename T, bool is_ad>
IndexableProperty<T, is_ad>::IndexableProperty(T * host,
                                               const std::string & property_param,
                                               const std::string & component_param)
  : _host(host),
    _property_param(property_param),
    _property_name(_host->template getParam<MaterialPropertyName>(_property_param)),
    _component_param(component_param),
    _component(host->template getParam<std::vector<unsigned int>>(_component_param)),
    _property_real(
        _host->template getGenericOptionalMaterialProperty<Real, is_ad>(_property_param)),
    _property_std_vector(
        _host->template getGenericOptionalMaterialProperty<std::vector<Real>, is_ad>(
            _property_param)),
    _property_real_vector_value(
        _host->template getGenericOptionalMaterialProperty<RealVectorValue, is_ad>(
            _property_param)),
    _property_rank_two_tensor(
        _host->template getGenericOptionalMaterialProperty<RankTwoTensor, is_ad>(_property_param)),
    _property_rank_three_tensor(
        _host->template getGenericOptionalMaterialProperty<RankThreeTensor, is_ad>(
            _property_param)),
    _property_rank_four_tensor(
        _host->template getGenericOptionalMaterialProperty<RankFourTensor, is_ad>(_property_param))
{
}

template <typename T, bool is_ad>
GenericReal<is_ad>
IndexableProperty<T, is_ad>::operator[](int qp) const
{
  if (_property_real)
    return _property_real[qp];
  if (_property_std_vector)
    return _property_std_vector[qp][_component[0]];
  if (_property_real_vector_value)
    return _property_real_vector_value[qp](_component[0]);
  if (_property_rank_two_tensor)
    return _property_rank_two_tensor[qp](_component[0], _component[1]);
  if (_property_rank_three_tensor)
    return _property_rank_three_tensor[qp](_component[0], _component[1], _component[2]);
  if (_property_rank_four_tensor)
    return _property_rank_four_tensor[qp](
        _component[0], _component[1], _component[2], _component[3]);
  _host->mooseError("internal error in IndexableProperty");
}

template <typename T, bool is_ad>
void
IndexableProperty<T, is_ad>::check() const
{
  if (_property_real)
    checkComponents(0);
  else if (_property_std_vector)
    checkComponents(1);
  else if (_property_real_vector_value)
    checkComponents(1);
  else if (_property_rank_two_tensor)
    checkComponents(2);
  else if (_property_rank_three_tensor)
    checkComponents(3);
  else if (_property_rank_four_tensor)
    checkComponents(4);
  else
    _host->mooseError("The ",
                      is_ad ? "AD" : "non-AD",
                      " material property '",
                      _property_name,
                      "' does not exist");
}

template <typename T, bool is_ad>
void
IndexableProperty<T, is_ad>::checkComponents(unsigned int components) const
{
  if (_component.size() != components)
    _host->mooseError("Material property '",
                      _property_name,
                      "' is ",
                      components,
                      "-dimensional, but an index vector of size ",
                      _component.size(),
                      " was supplied to select a component. It looks like you were expecting the "
                      "material property to have a different type.");
}
