//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ViscosityIC.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("SubChannelApp", ViscosityIC);

InputParameters
ViscosityIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription(
      "Computes viscosity of coolant based on initial conditions of pressure and temperature");
  params.addRequiredCoupledVar("T", "Temperature [K]");
  params.addRequiredParam<Real>("p", "Pressure [Pa]");
  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object name");
  params.addClassDescription("Computes viscosity from specified pressure and temperature");
  return params;
}

ViscosityIC::ViscosityIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _T(coupledValue("T")),
    _P(getParam<Real>("p")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
ViscosityIC::value(const Point & /*p*/)
{
  return _fp.mu_from_p_T(_P, _T[_qp]);
}
