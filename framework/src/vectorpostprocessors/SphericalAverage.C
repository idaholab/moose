//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SphericalAverage.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", SphericalAverage);

template <>
InputParameters
validParams<SphericalAverage>()
{
  InputParameters params = validParams<SpatialAverageBase>();
  return params;
}

SphericalAverage::SphericalAverage(const InputParameters & parameters)
  : SpatialAverageBase(parameters)
{
}

Real
SphericalAverage::computeDistance()
{
  // overwrite this method to implement cylindrical averages etc.
  return (_q_point[_qp] - _origin).norm();
}
