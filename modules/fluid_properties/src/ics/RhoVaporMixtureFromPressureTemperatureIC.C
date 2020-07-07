//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RhoVaporMixtureFromPressureTemperatureIC.h"

registerMooseObject("FluidPropertiesApp", RhoVaporMixtureFromPressureTemperatureIC);

InputParameters
RhoVaporMixtureFromPressureTemperatureIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params += VaporMixtureInterface<>::validParams();

  params.addClassDescription(
      "Computes the density of a vapor mixture from pressure and temperature.");

  params.addRequiredCoupledVar("p", "Pressure of the mixture [Pa]");
  params.addRequiredCoupledVar("T", "Temperature of the mixture [K]");

  return params;
}

RhoVaporMixtureFromPressureTemperatureIC::RhoVaporMixtureFromPressureTemperatureIC(
    const InputParameters & parameters)
  : VaporMixtureInterface<InitialCondition>(parameters),
    _p(coupledValue("p")),
    _T(coupledValue("T"))
{
}

Real
RhoVaporMixtureFromPressureTemperatureIC::value(const Point & /*p*/)
{
  const auto x = getMassFractionVector();
  return _fp_vapor_mixture.rho_from_p_T(_p[_qp], _T[_qp], x);
}
