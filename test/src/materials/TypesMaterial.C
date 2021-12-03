//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TypesMaterial.h"
#include "RankTwoTensor.h"
#include "RankThreeTensor.h"
#include "RankFourTensor.h"

#include "libmesh/int_range.h"

registerMooseObject("MooseTestApp", TypesMaterial);

InputParameters
TypesMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>(
      "std_vec_prop_entry1", 8.0, "Select a custom entry for _std_vec_prop[_qp](1)");
  return params;
}

TypesMaterial::TypesMaterial(const InputParameters & parameters)
  : Material(parameters),
    _real_prop(declareProperty<Real>("real_prop")),
    _std_vec_prop(declareProperty<std::vector<Real>>("stdvec_prop")),
    _std_vec_prop_entry1(getParam<Real>("std_vec_prop_entry1")),
    _std_vec_prop_qp(declareProperty<std::vector<Real>>("stdvec_prop_qp")),
    _std_vec_grad_prop(declareProperty<std::vector<RealGradient>>("stdvec_grad_prop")),
    _real_vec_prop(declareProperty<RealVectorValue>("realvec_prop")),
    _real_gradient_prop(declareProperty<RealGradient>("real_gradient_prop")),
    _matrix_prop(declareProperty<DenseMatrix<Real>>("matrix_prop")),
    _tensor_prop(declareProperty<RealTensorValue>("tensor_prop")),
    _rank_two_tensor_prop(declareProperty<RankTwoTensor>("rank_two_tensor_prop")),
    _rank_three_tensor_prop(declareProperty<RankThreeTensor>("rank_three_tensor_prop")),
    _rank_four_tensor_prop(declareProperty<RankFourTensor>("rank_four_tensor_prop"))
{
}

void
TypesMaterial::computeQpProperties()
{
  _real_prop[_qp] = 1;

  _std_vec_prop[_qp].resize(2);
  _std_vec_prop[_qp][0] = 9;
  _std_vec_prop[_qp][1] = _std_vec_prop_entry1;

  _std_vec_prop_qp[_qp].resize(1);
  _std_vec_prop_qp[_qp][0] = 1.;
  if (_qp == 0)
    _std_vec_prop_qp[_qp][0] = 0.;

  _std_vec_grad_prop[_qp].resize(2);
  _std_vec_grad_prop[_qp][0](0) = 2;
  _std_vec_grad_prop[_qp][0](1) = 5;
  _std_vec_grad_prop[_qp][0](2) = 7;
  _std_vec_grad_prop[_qp][1](0) = 10;
  _std_vec_grad_prop[_qp][1](1) = 12;
  _std_vec_grad_prop[_qp][1](2) = 15;

  _real_vec_prop[_qp](0) = 6;
  _real_vec_prop[_qp](1) = 5;
  _real_vec_prop[_qp](2) = 4;

  _matrix_prop[_qp].resize(1, 2);
  _matrix_prop[_qp](0, 0) = 20;
  _matrix_prop[_qp](0, 1) = 21;

  _tensor_prop[_qp](0, 0) = 11;
  _tensor_prop[_qp](1, 1) = 22;
  _tensor_prop[_qp](2, 2) = 33;

  _tensor_prop[_qp](0, 0) = 11;
  _tensor_prop[_qp](1, 1) = 22;
  _tensor_prop[_qp](2, 2) = 33;

  for (auto i : make_range(3))
  {
    _real_gradient_prop[_qp](i) = i;
    for (auto j : make_range(3))
    {
      _rank_two_tensor_prop[_qp](i, j) = i + 0.1 * j;
      for (auto k : make_range(3))
      {
        _rank_three_tensor_prop[_qp](i, j, k) = 10 * i + j + 0.1 * k;
        for (auto l : make_range(3))
          _rank_four_tensor_prop[_qp](i, j, k, l) = 100 * i + 10 * j + k + 0.1 * l;
      }
    }
  }
}
