//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProjectedStatefulPropertiesTestMaterial.h"

registerMooseObject("TensorMechanicsTestApp", ProjectedStatefulPropertiesTestMaterial);

template <bool is_ad>
InputParameters
ProjectedStatefulPropertiesTestMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("");
  return params;
}

template <bool is_ad>
ProjectedStatefulPropertiesTestMaterialTempl<is_ad>::ProjectedStatefulPropertiesTestMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    InterpolatedStatefulMaterialPropertyInterface(this),
    // Real
    _prop_real(declareGenericProperty<Real, is_ad>("test_real")),
    _prop_real_old(getMaterialPropertyOldByName<Real>("test_real")),
    _prop_real_old_interpolated(getInterpolatedMaterialPropertyOldByName<Real>("test_real")),
    // RealVectorValue
    _prop_realvectorvalue(declareGenericProperty<RealVectorValue, is_ad>("test_realvectorvalue")),
    _prop_realvectorvalue_old(
        getMaterialPropertyOldByName<RealVectorValue>("test_realvectorvalue")),
       _prop_realvectorvalue_old_interpolated(
         getInterpolatedMaterialPropertyOldByName<RealVectorValue>("test_realvectorvalue")),
    // RankTwoTensor
    _prop_ranktwotensor(declareGenericProperty<RankTwoTensor, is_ad>("test_ranktwotensor")),
    _prop_ranktwotensor_old(getMaterialPropertyOldByName<RankTwoTensor>("test_ranktwotensor")),
    _prop_ranktwotensor_old_interpolated(
        getInterpolatedMaterialPropertyOldByName<RankTwoTensor>("test_ranktwotensor")),
    // RankFourTensor
    _prop_rankfourtensor(declareGenericProperty<RankFourTensor, is_ad>("test_rankfourtensor")),
    _prop_rankfourtensor_old(getMaterialPropertyOldByName<RankFourTensor>("test_rankfourtensor")),
    _prop_rankfourtensor_old_interpolated(
        getInterpolatedMaterialPropertyOldByName<RankFourTensor>("test_rankfourtensor")),
    // diagnostic outputs
    _diff_norm(declareProperty<Real>("diff_norm"))
{
}

template <bool is_ad>
void
ProjectedStatefulPropertiesTestMaterialTempl<is_ad>::initQpStatefulProperties()
{
  _prop_real[_qp] = 77;
  _prop_realvectorvalue[_qp] = {2.4, 5.6, 7.8};
  _prop_ranktwotensor[_qp] = {1.1, 1.2, 1.3, 2.1, 2.2, 2.3, 3.1, 3.2, 3.3};
}

template <bool is_ad>
void
ProjectedStatefulPropertiesTestMaterialTempl<is_ad>::computeQpProperties()
{
  const auto & p = _q_point[_qp];

  // update current properties
  _prop_real[_qp] = 1.23 + _t * p(0) - p(1) * p(1);
  _prop_realvectorvalue[_qp] = _prop_realvectorvalue_old[_qp] + p;
  _prop_ranktwotensor[_qp] = _prop_ranktwotensor_old[_qp];
  _prop_rankfourtensor[_qp] = _prop_rankfourtensor_old[_qp];

  for (const auto i : make_range(Moose::dim))
  {
    _prop_realvectorvalue[_qp](i) += MathUtils::pow(_t, i);
    for (const auto j : make_range(Moose::dim))
    {
      _prop_ranktwotensor[_qp](i, j) += MathUtils::pow(_t, i) + MathUtils::pow(p(0), j);
      for (const auto k : make_range(Moose::dim))
      {
        for (const auto l : make_range(Moose::dim))
          _prop_rankfourtensor[_qp](i, j, k, l) += MathUtils::pow(_t, i) + MathUtils::pow(p(2), j) +
                                                   MathUtils::pow(p(1), k) +
                                                   MathUtils::pow(p(0), l);
      }
    }
  }

  // compute norm of old diffs
  const auto d1 = _prop_real_old[_qp] - _prop_real_old_interpolated[_qp];
  const auto d2 = _prop_realvectorvalue_old[_qp] - _prop_realvectorvalue_old_interpolated[_qp];
  const auto d3 = _prop_ranktwotensor_old[_qp] - _prop_ranktwotensor_old_interpolated[_qp];
  const auto d4 = _prop_rankfourtensor_old[_qp] - _prop_rankfourtensor_old_interpolated[_qp];

  Real diff = 0.0;
  diff += std::sqrt(d1 * d1);
  diff += d2.norm();
  diff += d3.L2norm();
  diff += d4.L2norm();
  _diff_norm[_qp] = diff;
}
