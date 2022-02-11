//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Sampler1DVector.h"

registerMooseObject("ThermalHydraulicsApp", Sampler1DVector);

InputParameters
Sampler1DVector::validParams()
{
  InputParameters params = Sampler1DBase<Real>::validParams();
  params.addRequiredParam<unsigned int>("index",
                                        "Index of the vector property component to sample");
  return params;
}

Sampler1DVector::Sampler1DVector(const InputParameters & parameters)
  : Sampler1DBase<std::vector<Real>>(parameters), _index(getParam<unsigned int>("index"))
{
}

Real
Sampler1DVector::getScalarFromProperty(const std::vector<Real> & property,
                                       const Point & /*curr_point*/)
{
  return property[_index];
}
