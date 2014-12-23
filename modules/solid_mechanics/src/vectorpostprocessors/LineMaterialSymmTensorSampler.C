/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "LineMaterialSymmTensorSampler.h"

template<>
InputParameters validParams<LineMaterialSymmTensorSampler>()
{
  InputParameters params = validParams<LineMaterialSamplerBase<Real> >();
  params += validParams<MaterialTensorCalculator>();
  return params;
}

LineMaterialSymmTensorSampler::LineMaterialSymmTensorSampler(const std::string & name, InputParameters parameters) :
    LineMaterialSamplerBase<SymmTensor>(name, parameters),
    MaterialTensorCalculator(name, parameters)
{
}

Real
LineMaterialSymmTensorSampler::getScalarFromProperty(SymmTensor &property, const Point * curr_point)
{
  RealVectorValue direction;
  return getTensorQuantity(property, curr_point, direction);
}
