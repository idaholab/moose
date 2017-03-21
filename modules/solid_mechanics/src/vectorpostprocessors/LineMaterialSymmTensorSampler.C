/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LineMaterialSymmTensorSampler.h"

template <>
InputParameters
validParams<LineMaterialSymmTensorSampler>()
{
  InputParameters params = validParams<LineMaterialSamplerBase<Real>>();
  params += validParams<MaterialTensorCalculator>();
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
