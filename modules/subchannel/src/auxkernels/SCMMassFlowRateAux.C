//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMMassFlowRateAux.h"

registerMooseObject("SubChannelApp", SCMMassFlowRateAux);

InputParameters
SCMMassFlowRateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes mass flow rate from specified mass flux and subchannel cross-sectional "
      "area. Can read either PostprocessorValue or Real");
  params.addRequiredCoupledVar("area", "Cross sectional area [m^2]");
  params.addRequiredParam<PostprocessorName>(
      "mass_flux", "The postprocessor or Real to use for the value of mass_flux");
  return params;
}

SCMMassFlowRateAux::SCMMassFlowRateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _mass_flux(getPostprocessorValue("mass_flux")),
    _area(coupledValue("area"))
{
}

Real
SCMMassFlowRateAux::computeValue()
{
  return _mass_flux * _area[_qp];
}
