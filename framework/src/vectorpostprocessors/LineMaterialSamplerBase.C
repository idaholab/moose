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

#include "LineMaterialSamplerBase.h"

template <>
InputParameters
validParams<LineMaterialSamplerBase<Real>>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  params += validParams<SamplerBase>();
  params += validParams<BlockRestrictable>();
  params.addRequiredParam<Point>("start", "The beginning of the line");
  params.addRequiredParam<Point>("end", "The end of the line");
  params.addRequiredParam<std::vector<std::string>>(
      "property", "Name of the material property to be output along a line");

  return params;
}
