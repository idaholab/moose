/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LineMaterialRankTwoSampler.h"
#include "RankTwoScalarTools.h"

template <>
InputParameters
validParams<LineMaterialRankTwoSampler>()
{
  InputParameters params = validParams<LineMaterialSamplerBase<Real>>();
  params.addClassDescription("Access a component of a RankTwoTensor");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_i",
      "index_i >= 0 & index_i <= 2",
      "The index i of ij for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_j",
      "index_j >= 0 & index_j <= 2",
      "The index j of ij for the tensor to output (0, 1, 2)");
  return params;
}

LineMaterialRankTwoSampler::LineMaterialRankTwoSampler(const InputParameters & parameters)
  : LineMaterialSamplerBase<RankTwoTensor>(parameters),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j"))
{
}

Real
LineMaterialRankTwoSampler::getScalarFromProperty(const RankTwoTensor & property,
                                                  const Point & /*curr_point*/)
{
  return RankTwoScalarTools::component(property, _i, _j);
}
