//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CylindricalRankTwoAux.h"

registerMooseObject("TensorMechanicsApp", CylindricalRankTwoAux);

InputParameters
CylindricalRankTwoAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Takes RankTwoTensor material and outputs component in cylindrical coordinates");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_i",
      "index_i >= 0 & index_i <= 2",
      "The index i of ij for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_j",
      "index_j >= 0 & index_j <= 2",
      "The index j of ij for the tensor to output (0, 1, 2)");
  params.addRequiredParam<Point>("center_point",
                                 "Location of the center point of the cylindrical coordinates");
  return params;
}

CylindricalRankTwoAux::CylindricalRankTwoAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor")),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j")),
    _center_point(getParam<Point>("center_point"))
{
}

Real
CylindricalRankTwoAux::computeValue()
{
  Point loc_from_center = _q_point[_qp] - _center_point;

  Real theta = std::atan2(loc_from_center(1), loc_from_center(0));
  RankTwoTensor R;
  R(0, 0) = std::cos(theta);
  R(0, 1) = std::sin(theta);
  R(1, 0) = -std::sin(theta);
  R(1, 1) = std::cos(theta);

  RankTwoTensor rotated_tensor = R * _tensor[_qp] * R.transpose();

  return rotated_tensor(_i, _j);
}
