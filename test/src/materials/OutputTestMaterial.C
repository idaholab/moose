//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OutputTestMaterial.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

registerMooseObject("MooseTestApp", OutputTestMaterial);
registerMooseObject("MooseTestApp", ADOutputTestMaterial);

template <bool is_ad>
InputParameters
OutputTestMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>(
      "real_property_name", "real_property", "The name of the scalar real property");
  params.addParam<std::string>(
      "vector_property_name", "vector_property", "The name of the vector real property");
  params.addParam<std::string>(
      "tensor_property_name", "tensor_property", "The name of the tensor real property");
  params.addParam<std::string>("ranktwotensor_property_name",
                               "ranktwotensor_property",
                               "The name of the rank two tensor property");
  params.addParam<std::string>("rankfourtensor_property_name",
                               "rankfourtensor_property",
                               "The name of the rank four tensor property");

  // variable names are limited to 32 characters
  params.addParam<std::string>("symmetricranktwotensor_property_name",
                               "symmetricr2t_property",
                               "The name of the symmetric rank two tensor property");
  params.addParam<std::string>("symmetricrankfourtensor_property_name",
                               "symmetricr4t_property",
                               "The name of the symmetric rank four tensor property");

  params.addParam<std::string>("stdvector_property_name",
                               "The name of the standard vector property");
  params.addParam<Real>(
      "real_factor", 0, "Add this factor to all of the scalar real material property");
  params.addCoupledVar("variable",
                       "Variable to use for making this test material more complicated");
  return params;
}

template <bool is_ad>
OutputTestMaterialTempl<is_ad>::OutputTestMaterialTempl(const InputParameters & parameters)
  : Material(parameters),
    _real_property(this->template declareGenericPropertyByName<Real, is_ad>(
        getParam<std::string>("real_property_name"))),
    _vector_property(this->template declareGenericPropertyByName<RealVectorValue, is_ad>(
        getParam<std::string>("vector_property_name"))),
    _tensor_property(this->template declareGenericPropertyByName<RealTensorValue, is_ad>(
        getParam<std::string>("tensor_property_name"))),
    _ranktwotensor_property(this->template declareGenericPropertyByName<RankTwoTensor, is_ad>(
        getParam<std::string>("ranktwotensor_property_name"))),
    _rankfourtensor_property(this->template declareGenericPropertyByName<RankFourTensor, is_ad>(
        getParam<std::string>("rankfourtensor_property_name"))),
    _symmetricranktwotensor_property(
        this->template declareGenericPropertyByName<SymmetricRankTwoTensor, is_ad>(
            getParam<std::string>("symmetricranktwotensor_property_name"))),
    _symmetricrankfourtensor_property(
        this->template declareGenericPropertyByName<SymmetricRankFourTensor, is_ad>(
            getParam<std::string>("symmetricrankfourtensor_property_name"))),
    _factor(getParam<Real>("real_factor")),
    _variable(coupledGenericValue<is_ad>("variable"))
{
  if (isParamValid("stdvector_property_name"))
    _stdvector_property = &(this->template declareGenericPropertyByName<std::vector<Real>, is_ad>(
        getParam<std::string>("stdvector_property_name")));
  else
    _stdvector_property = nullptr;
}

template <bool is_ad>
OutputTestMaterialTempl<is_ad>::~OutputTestMaterialTempl()
{
}

template <bool is_ad>
void
OutputTestMaterialTempl<is_ad>::computeQpProperties()
{
  Point p = _q_point[_qp];
  GenericReal<is_ad> v = std::floor(_variable[_qp] + 0.5);
  GenericReal<is_ad> x = std::ceil(p(0));
  GenericReal<is_ad> y = std::ceil(p(1));

  _real_property[_qp] = v + y + x + _factor;

  GenericRealVectorValue<is_ad> vec(v + x, v + y);
  _vector_property[_qp] = vec;

  GenericRealTensorValue<is_ad> tensor(v, x * y, 0, -x * y, y * y);
  _tensor_property[_qp] = tensor;

  _ranktwotensor_property[_qp] = tensor;

  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      for (const auto k : make_range(Moose::dim))
        for (const auto l : make_range(Moose::dim))
          _rankfourtensor_property[_qp](i, j, k, l) =
              i * 1000 + j * 100 + k * 10 + l + _variable[_qp] / 10.0;

  for (const auto i : make_range(SymmetricRankTwoTensor::N))
    _symmetricranktwotensor_property[_qp](i) = i + _variable[_qp] / 10.0;

  for (const auto i : make_range(SymmetricRankFourTensor::N))
    for (const auto j : make_range(SymmetricRankFourTensor::N))
      _symmetricrankfourtensor_property[_qp](i, j) = i * 10 + j + _variable[_qp] / 10.0;

  if (_stdvector_property)
  {
    (*_stdvector_property)[_qp].resize(2);
    (*_stdvector_property)[_qp][0] = vec(0);
    (*_stdvector_property)[_qp][1] = vec(1);
  }
}

template class OutputTestMaterialTempl<false>;
template class OutputTestMaterialTempl<true>;
