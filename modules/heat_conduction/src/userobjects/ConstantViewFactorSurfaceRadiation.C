//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantViewFactorSurfaceRadiation.h"

registerMooseObject("HeatConductionApp", ConstantViewFactorSurfaceRadiation);

InputParameters
ConstantViewFactorSurfaceRadiation::validParams()
{
  InputParameters params = GrayLambertSurfaceRadiationBase::validParams();
  params.addRequiredParam<std::vector<std::vector<Real>>>(
      "view_factors", "The view factors from sideset i to sideset j.");
  params.addClassDescription(
      "ConstantViewFactorSurfaceRadiation computes radiative heat transfer between side sets and "
      "the view factors are provided in the input file");
  return params;
}

ConstantViewFactorSurfaceRadiation::ConstantViewFactorSurfaceRadiation(
    const InputParameters & parameters)
  : GrayLambertSurfaceRadiationBase(parameters)
{
}

std::vector<std::vector<Real>>
ConstantViewFactorSurfaceRadiation::setViewFactors()
{
  std::vector<std::vector<Real>> vf = getParam<std::vector<std::vector<Real>>>("view_factors");

  // check that the input has the right format
  if (vf.size() != _n_sides)
    paramError("view_factors",
               "Leading dimension of view_factors must be equal to number of side sets.");

  for (unsigned int i = 0; i < _n_sides; ++i)
    if (vf[i].size() != _n_sides)
      paramError("view_factors",
                 "view_factors must be provided as square array. Row ",
                 i,
                 " has ",
                 vf[i].size(),
                 " entries.");
  return vf;
}

void
ConstantViewFactorSurfaceRadiation::initialize()
{
  GrayLambertSurfaceRadiationBase::initialize();

  // check row-sum and normalize if necessary
  for (unsigned int i = 0; i < _n_sides; ++i)
  {
    Real sum = 0;
    for (auto & v : _view_factors[i])
      sum += v;

    // an error of 5% is acceptable, but more indicates an error in the
    // problem setup
    if (std::abs(sum - 1) > 0.05)
      mooseError("view_factors row ", i, " sums to ", sum);

    // correct view factors
    for (auto & v : _view_factors[i])
      v /= sum;
  }
}
