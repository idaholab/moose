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

template <typename T, bool is_ad>
class IndexableProperty
{
public:
  static InputParameters validParams();

  IndexableProperty(T * host,
                    const std::string & property_param = "property",
                    const std::string & component_param = "component");

  GenericReal<is_ad> operator[](int i) const;
  operator bool() const;

protected:
  template <typename P>
  void getPropertyHelper(const GenericMaterialProperty<P, is_ad> *& pointer,
                         unsigned int components);

  const GenericMaterialProperty<Real, is_ad> * _property_real;
  const GenericMaterialProperty<std::vector<Real>, is_ad> * _property_std_vector;
  const GenericMaterialProperty<RealVectorValue, is_ad> * _property_real_vector_value;
  const GenericMaterialProperty<RankTwoTensor, is_ad> * _property_rank_two_tensor;
  const GenericMaterialProperty<RankThreeTensor, is_ad> * _property_rank_three_tensor;
  const GenericMaterialProperty<RankFourTensor, is_ad> * _property_rank_four_tensor;

  T * _host;

  const std::string & _property_param;
  const std::string & _property_name;
  const std::string & _component_param;

  const std::vector<unsigned int> _component;
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
    _component(host->template getParam<std::vector<unsigned int>>(_component_param))
{
  getPropertyHelper<Real>(_property_real, 0);
  getPropertyHelper<RealVectorValue>(_property_real_vector_value, 1);
  getPropertyHelper<std::vector<Real>>(_property_std_vector, 1);
  getPropertyHelper<RankTwoTensor>(_property_rank_two_tensor, 2);
  getPropertyHelper<RankThreeTensor>(_property_rank_three_tensor, 3);
  getPropertyHelper<RankFourTensor>(_property_rank_four_tensor, 4);
}

template <typename T, bool is_ad>
GenericReal<is_ad>
IndexableProperty<T, is_ad>::operator[](int i) const
{
  if (_property_real)
    return (*_property_real)[i];
  if (_property_std_vector)
    return (*_property_std_vector)[i][_component[0]];
  if (_property_real_vector_value)
    return (*_property_real_vector_value)[i](_component[0]);
  if (_property_rank_two_tensor)
    return (*_property_rank_two_tensor)[i](_component[0], _component[1]);
  if (_property_rank_three_tensor)
    return (*_property_rank_three_tensor)[i](_component[0], _component[1], _component[2]);
  if (_property_rank_four_tensor)
    return (*_property_rank_four_tensor)[i](
        _component[0], _component[1], _component[2], _component[3]);
  mooseError(
      "The ", is_ad ? "AD" : "non-AD", " material property '", _property_name, "' does not exist");
}

template <typename T, bool is_ad>
IndexableProperty<T, is_ad>::operator bool() const
{
  return _property_real || _property_std_vector || _property_real_vector_value ||
         _property_rank_two_tensor || _property_rank_three_tensor || _property_rank_four_tensor;
}

template <typename T, bool is_ad>
template <typename P>
void
IndexableProperty<T, is_ad>::getPropertyHelper(const GenericMaterialProperty<P, is_ad> *& pointer,
                                               unsigned int components)
{
  if (_host->template hasGenericMaterialProperty<P, is_ad>(_property_param))
  {
    pointer = &_host->template getGenericMaterialProperty<P, is_ad>(_property_param);

    if (_component.size() != components)
      mooseError("Material property '",
                 _property_name,
                 "' is ",
                 components,
                 " dimensional, but an index vector of size ",
                 _component.size(),
                 " was supplied to select a component. It looks like you were expecting the "
                 "material property to have a different type.");
  }
  else
    pointer = nullptr;
}
