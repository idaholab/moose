//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProjectedStatefulPropertiesTestMaterial.h"

registerMooseObject("MooseTestApp", ProjectedStatefulPropertiesTestMaterial);

template <bool is_ad>
InputParameters
ProjectedStatefulPropertiesTestMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Test material for projected stateful properties");
  params.set<bool>("use_interpolated_state") = true;
  return params;
}

template <bool is_ad>
ProjectedStatefulPropertiesTestMaterialTempl<is_ad>::ProjectedStatefulPropertiesTestMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    // Real
    _prop_real(declareGenericProperty<Real, is_ad>("test_real")),
    _prop_real_old(getMaterialPropertyOldByName<Real>("test_real")),
    // RealVectorValue
    _prop_realvectorvalue(declareGenericProperty<RealVectorValue, is_ad>("test_realvectorvalue")),
    _prop_realvectorvalue_old(
        getMaterialPropertyOldByName<RealVectorValue>("test_realvectorvalue")),
    // RankTwoTensor
    _prop_ranktwotensor(declareGenericProperty<RankTwoTensor, is_ad>("test_ranktwotensor")),
    _prop_ranktwotensor_old(getMaterialPropertyOldByName<RankTwoTensor>("test_ranktwotensor")),
    // RankFourTensor
    _prop_rankfourtensor(declareGenericProperty<RankFourTensor, is_ad>("test_rankfourtensor")),
    _prop_rankfourtensor_old(getMaterialPropertyOldByName<RankFourTensor>("test_rankfourtensor")),
    // diagnostic outputs
    _diff_norm(declareProperty<Real>("diff_norm"))
{
}

template <bool is_ad>
void
ProjectedStatefulPropertiesTestMaterialTempl<is_ad>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <bool is_ad>
void
ProjectedStatefulPropertiesTestMaterialTempl<is_ad>::computeQpProperties()
{
  const auto & p = _q_point[_qp];

  auto v_real = [&p](Real t) { return 1.23 + t * p(0) - p(1) * p(1); };
  auto v_realvectorvalue = [&p](Real t)
  {
    RealVectorValue r = p;
    for (const auto i : make_range(Moose::dim))
      r(i) += MathUtils::pow(t, i);
    return r;
  };
  auto v_ranktwotensor = [&p](Real t)
  {
    RankTwoTensor r;
    for (const auto i : make_range(Moose::dim))
      for (const auto j : make_range(Moose::dim))
        r(i, j) += MathUtils::pow(t, i) + MathUtils::pow(p(i), j);
    return r;
  };
  auto v_rankfourtensor = [&p](Real t)
  {
    RankFourTensor r;
    for (const auto i : make_range(Moose::dim))
      for (const auto j : make_range(Moose::dim))
        for (const auto k : make_range(Moose::dim))
          for (const auto l : make_range(Moose::dim))
            r(i, j, k, l) += MathUtils::pow(t, i) + MathUtils::pow(p(2), j) +
                             MathUtils::pow(p(1), k) + MathUtils::pow(p(0), l);
    return r;
  };

  // update current properties
  _prop_real[_qp] = v_real(_t);
  _prop_realvectorvalue[_qp] = v_realvectorvalue(_t);
  _prop_ranktwotensor[_qp] = v_ranktwotensor(_t);
  _prop_rankfourtensor[_qp] = v_rankfourtensor(_t);

  // compute norm of old diffs
  const auto d1 = _prop_real_old[_qp] - v_real(_t - _dt);
  const auto d2 = _prop_realvectorvalue_old[_qp] - v_realvectorvalue(_t - _dt);
  const auto d3 = _prop_ranktwotensor_old[_qp] - v_ranktwotensor(_t - _dt);
  const auto d4 = _prop_rankfourtensor_old[_qp] - v_rankfourtensor(_t - _dt);

  Real diff = 0.0;
  diff += std::sqrt(d1 * d1);
  diff += d2.norm();
  diff += d3.L2norm();
  diff += d4.L2norm();
  _diff_norm[_qp] = diff;
}
