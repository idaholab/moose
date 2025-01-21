//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMMassFlowRateIC.h"

registerMooseObject("SubChannelApp", SCMMassFlowRateIC);

InputParameters
SCMMassFlowRateIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription(
      "Computes mass float rate from specified mass flux and cross-sectional area");
  params.addRequiredCoupledVar("area", "Subchannel surface area [m^2]");
  params.addRequiredParam<Real>("mass_flux", "Specified mass flux [kg/s-m^2]");
  return params;
}

SCMMassFlowRateIC::SCMMassFlowRateIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _mass_flux(getParam<Real>("mass_flux")),
    _area(coupledValue("area"))
{
}

Real
SCMMassFlowRateIC::value(const Point & /*p*/)
{
  return _mass_flux * _area[_qp];
}
