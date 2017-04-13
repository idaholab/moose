/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LineMaterialRankTwoScalarSampler.h"
#include "RankTwoScalarTools.h"

template <>
InputParameters
validParams<LineMaterialRankTwoScalarSampler>()
{
  InputParameters params = validParams<LineMaterialSamplerBase<Real>>();
  params.addClassDescription("Compute a scalar property of a RankTwoTensor");
  params.addParam<MooseEnum>(
      "scalar_type", RankTwoScalarTools::scalarOptions(), "A scalar to ouput");
  params.addParam<Point>(
      "point1",
      Point(0, 0, 0),
      "Start point for axis used to calculate some cylinderical material tensor quantities");
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
