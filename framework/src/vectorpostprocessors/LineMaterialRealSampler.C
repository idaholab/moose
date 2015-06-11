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

#include "LineMaterialRealSampler.h"

template<>
InputParameters validParams<LineMaterialRealSampler>()
{
  InputParameters params = validParams<LineMaterialSamplerBase<Real> >();
  return params;
}

LineMaterialRealSampler::LineMaterialRealSampler(const std::string & name, InputParameters parameters) :
    LineMaterialSamplerBase<Real>(name, parameters)
{
}

Real
LineMaterialRealSampler::getScalarFromProperty(const Real & property, const Point * /*curr_point*/)
{
  return property;
}
