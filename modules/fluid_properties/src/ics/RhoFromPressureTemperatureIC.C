//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RhoFromPressureTemperatureIC.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", RhoFromPressureTemperatureIC);

InputParameters
RhoFromPressureTemperatureIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object.");
  params.addRequiredCoupledVar("p", "The pressure [Pa]");
  params.addRequiredCoupledVar("T", "The temperature [K]");
  params.addClassDescription("Computes the density from pressure and temperature.");
  return params;
}

RhoFromPressureTemperatureIC::RhoFromPressureTemperatureIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _spfp(getUserObject<SinglePhaseFluidProperties>("fp")),
    _p(coupledValue("p")),
    _T(coupledValue("T"))
{
}

Real
RhoFromPressureTemperatureIC::value(const Point & /*p*/)
{
  return _spfp.rho_from_p_T(_p[_qp], _T[_qp]);
}
