//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LineMaterialSymmTensorSampler.h"

registerMooseObject("SolidMechanicsApp", LineMaterialSymmTensorSampler);

InputParameters
LineMaterialSymmTensorSampler::validParams()
{
  InputParameters params = LineMaterialSamplerBase<Real>::validParams();
  params += MaterialTensorCalculator::validParams();
  return params;
}

LineMaterialSymmTensorSampler::LineMaterialSymmTensorSampler(const InputParameters & parameters)
  : LineMaterialSamplerBase<SymmTensor>(parameters), MaterialTensorCalculator(parameters)
{
}

Real
LineMaterialSymmTensorSampler::getScalarFromProperty(const SymmTensor & property,
                                                     const Point & curr_point)
{
  RealVectorValue direction;
  return getTensorQuantity(property, curr_point, direction);
}
