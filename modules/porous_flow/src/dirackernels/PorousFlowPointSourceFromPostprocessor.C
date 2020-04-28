//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPointSourceFromPostprocessor.h"

registerMooseObject("PorousFlowApp", PorousFlowPointSourceFromPostprocessor);

InputParameters
PorousFlowPointSourceFromPostprocessor::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params.addRequiredParam<PostprocessorName>(
      "mass_flux",
      "The postprocessor name holding the mass flux at this point in kg/s (positive is flux in, "
      "negative is flux out)");
  params.addRequiredParam<Point>("point", "The x,y,z coordinates of the point source (or sink)");
  params.addClassDescription(
      "Point source (or sink) that adds (or removes) fluid at a mass flux rate "
      "specified by a postprocessor.");
  return params;
}

PorousFlowPointSourceFromPostprocessor::PorousFlowPointSourceFromPostprocessor(
    const InputParameters & parameters)
  : DiracKernel(parameters),
    _mass_flux(getPostprocessorValue("mass_flux")),
    _p(getParam<Point>("point"))
{
}

void
PorousFlowPointSourceFromPostprocessor::addPoints()
{
  addPoint(_p, 0);
}

Real
PorousFlowPointSourceFromPostprocessor::computeQpResidual()
{
  // Negative sign to make a positive mass_flux in the input file a source
  return -_test[_i][_qp] * _mass_flux;
}
