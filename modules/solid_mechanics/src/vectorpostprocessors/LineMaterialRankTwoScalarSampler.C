//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LineMaterialRankTwoScalarSampler.h"
#include "RankTwoScalarTools.h"

registerMooseObject("TensorMechanicsApp", LineMaterialRankTwoScalarSampler);

InputParameters
LineMaterialRankTwoScalarSampler::validParams()
{
  InputParameters params = LineMaterialSamplerBase<Real>::validParams();
  params.addClassDescription("Compute a scalar property of a RankTwoTensor");
  params.addParam<MooseEnum>(
      "scalar_type", RankTwoScalarTools::scalarOptions(), "A scalar to output");
  params.addParam<Point>(
      "point1",
      Point(0, 0, 0),
      "Start point for axis used to calculate some cylindrical material tensor quantities");
  params.addParam<Point>("point2",
                         Point(0, 1, 0),
                         "End point for axis used to calculate some material tensor quantities");
  params.addParam<Point>("direction", Point(0, 0, 1), "Direction vector");
  return params;
}

LineMaterialRankTwoScalarSampler::LineMaterialRankTwoScalarSampler(
    const InputParameters & parameters)
  : LineMaterialSamplerBase<RankTwoTensor>(parameters),
    _scalar_type(getParam<MooseEnum>("scalar_type")),
    _point1(parameters.get<Point>("point1")),
    _point2(parameters.get<Point>("point2")),
    _direction(parameters.get<Point>("direction") / parameters.get<Point>("direction").norm())
{
}

Real
LineMaterialRankTwoScalarSampler::getScalarFromProperty(const RankTwoTensor & property,
                                                        const Point & curr_point)
{
  return RankTwoScalarTools::getQuantity(
      property, _scalar_type, _point1, _point2, curr_point, _direction);
}
