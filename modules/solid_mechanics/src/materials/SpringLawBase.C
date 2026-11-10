//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpringLawBase.h"

InputParameters
SpringLawBase::validParams()
{
  InputParameters params = Material::validParams();
  MooseEnum mode("translational rotational", "translational");
  params.addParam<MooseEnum>(
      "mode",
      mode,
      "Which relative motion the law acts on: the three translations of node 1 with respect to "
      "node 0 in the local frame (translational, the default, giving spring_force), or the "
      "three rotations (rotational, giving spring_moment). A spring with both takes one law of "
      "each mode on the same block.");
  params.addParam<std::string>(
      "base_name",
      "The base_name of the ComputeSpringDeformation material of this spring, when several "
      "springs share a block; the law reads base_name_spring_deformation and writes "
      "base_name_spring_force.");
  params.addRangeCheckedParam<Real>(
      "tributary_area",
      1.0,
      "tributary_area > 0",
      "Multiplier of every force of the law, the area of the interface the spring represents "
      "(dimension length squared). With the default 1 the law coefficients are those of the "
      "spring itself, a stiffness in force per length; with the area they are given per unit "
      "area, a stiffness in force per length cubed (a subgrade modulus) and a strength in "
      "force per area, and the same law serves a point spring, a bed of springs and a pile "
      "shaft.");
  return params;
}

SpringLawBase::SpringLawBase(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _rotational(getParam<MooseEnum>("mode") == "rotational"),
    _tributary_area(getParam<Real>("tributary_area")),
    _deformation(getMaterialPropertyByName<RealVectorValue>(
        _base_name + (_rotational ? "spring_rotation" : "spring_deformation"))),
    _force(declareProperty<RealVectorValue>(_base_name +
                                            (_rotational ? "spring_moment" : "spring_force"))),
    _stiffness(declareProperty<RankTwoTensor>(
        _base_name + (_rotational ? "spring_rotational_stiffness" : "spring_stiffness")))
{
}

void
SpringLawBase::computeQpProperties()
{
  RealVectorValue force;
  RankTwoTensor stiffness;
  computeLaw(_deformation[_qp], force, stiffness);
  _force[_qp] = _tributary_area * force;
  _stiffness[_qp] = _tributary_area * stiffness;
}
