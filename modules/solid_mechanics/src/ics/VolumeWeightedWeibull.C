//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeWeightedWeibull.h"

registerMooseObject("TensorMechanicsApp", VolumeWeightedWeibull);

InputParameters
VolumeWeightedWeibull::validParams()
{
  InputParameters params = RandomICBase::validParams();
  params.addRequiredParam<Real>("reference_volume", "Reference volume (of a test specimen)");
  params.addRequiredParam<Real>("weibull_modulus", "Weibull modulus");
  params.addParam<Real>(
      "median",
      "Median value of property measured in a specimen of volume equal to reference_volume");
  params.addClassDescription("Initialize a variable with randomly generated numbers following "
                             "a volume-weighted Weibull distribution");
  return params;
}

VolumeWeightedWeibull::VolumeWeightedWeibull(const InputParameters & parameters)
  : RandomICBase(parameters),
    _reference_volume(getParam<Real>("reference_volume")),
    _weibull_modulus(getParam<Real>("weibull_modulus")),
    _median(getParam<Real>("median"))
{
}

Real
VolumeWeightedWeibull::value(const Point & /*p*/)
{
  return _median * std::pow(_reference_volume * std::log(generateRandom()) /
                                (_current_elem_volume * std::log(0.5)),
                            1.0 / _weibull_modulus);
}
