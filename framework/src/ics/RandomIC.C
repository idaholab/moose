//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RandomIC.h"

#include "libmesh/point.h"
#include "Distribution.h"

registerMooseObject("MooseApp", RandomIC);

InputParameters
RandomIC::validParams()
{
  InputParameters params = RandomICBase::validParams();
  params += DistributionInterface::validParams();
  params.addParam<Real>(
      "min", 0.0, "Lower bound of uniformly distributed randomly generated values");
  params.addParam<Real>(
      "max", 1.0, "Upper bound of uniformly distributed randomly generated values");
  params.addParam<DistributionName>(
      "distribution", "Name of distribution defining distribution of randomly generated values");

  params.addClassDescription("Initialize a variable with randomly generated numbers following "
                             "either a uniform distribution or a user-defined distribution");
  return params;
}

RandomIC::RandomIC(const InputParameters & parameters)
  : RandomICBase(parameters),
    DistributionInterface(this),
    _min(getParam<Real>("min")),
    _max(getParam<Real>("max")),
    _distribution(nullptr)
{
  if (_min >= _max)
    paramError("min", "Min >= Max for RandomIC!");

  if (parameters.isParamSetByUser("distribution"))
  {
    _distribution = &getDistributionByName(getParam<DistributionName>("distribution"));
    if (parameters.isParamSetByUser("min") || parameters.isParamSetByUser("max"))
      paramError("distribution", "Cannot use together with 'min' or 'max' parameter");
  }
}

Real
RandomIC::value(const Point & /*p*/)
{
  if (_distribution)
    return _distribution->quantile(generateRandom());
  else
    return generateRandom() * (_max - _min) + _min;
}
