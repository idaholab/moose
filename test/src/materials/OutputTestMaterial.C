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

registerMooseObject("MooseTestApp", OutputTestMaterial);

InputParameters
OutputTestMaterial::validParams()
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
  params.addParam<std::string>("stdvector_property_name",
                               "The name of the standard vector property");
  params.addParam<Real>(
      "real_factor", 0, "Add this factor to all of the scalar real material property");
  params.addCoupledVar("variable",
                       "Variable to use for making this test material more complicated");
  return params;
}

OutputTestMaterial::OutputTestMaterial(const InputParameters & parameters)
  : Material(parameters),
    _real_property(declareProperty<Real>(getParam<std::string>("real_property_name"))),
    _vector_property(
        declareProperty<RealVectorValue>(getParam<std::string>("vector_property_name"))),
    _tensor_property(
        declareProperty<RealTensorValue>(getParam<std::string>("tensor_property_name"))),
    _ranktwotensor_property(
        declareProperty<RankTwoTensor>(getParam<std::string>("ranktwotensor_property_name"))),
    _rankfourtensor_property(
        declareProperty<RankFourTensor>(getParam<std::string>("rankfourtensor_property_name"))),
    _factor(getParam<Real>("real_factor")),
    _variable(coupledValue("variable"))
{
  if (isParamValid("stdvector_property_name"))
    _stdvector_property =
        &declareProperty<std::vector<Real>>(getParam<std::string>("stdvector_property_name"));
  else
    _stdvector_property = nullptr;
}

OutputTestMaterial::~OutputTestMaterial() {}

void
OutputTestMaterial::computeQpProperties()
{
  Point p = _q_point[_qp];
  Real v = std::floor(_variable[_qp] + 0.5);
  Real x = std::ceil(p(0));
  Real y = std::ceil(p(1));

  _real_property[_qp] = v + y + x + _factor;

  RealVectorValue vec(v + x, v + y);
  _vector_property[_qp] = vec;

  RealTensorValue tensor(v, x * y, 0, -x * y, y * y);
  _tensor_property[_qp] = tensor;

  _ranktwotensor_property[_qp] = tensor;

  RankFourTensor rankfourtensor;
  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      for (const auto k : make_range(Moose::dim))
        for (const auto l : make_range(Moose::dim))
          _rankfourtensor_property[_qp](i, j, k, l) =
              i * 1000 + j * 100 + k * 10 + l + _variable[_qp] / 10.0;

  if (_stdvector_property)
  {
    (*_stdvector_property)[_qp].resize(2);
    (*_stdvector_property)[_qp][0] = vec(0);
    (*_stdvector_property)[_qp][1] = vec(1);
  }
}
