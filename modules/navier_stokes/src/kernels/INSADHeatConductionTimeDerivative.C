//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADHeatConductionTimeDerivative.h"

registerMooseObject("HeatConductionApp", INSADHeatConductionTimeDerivative);

InputParameters
INSADHeatConductionTimeDerivative::validParams()
{
  InputParameters params = ADTimeDerivative::validParams();
  params.addClassDescription(
      "AD Time derivative term $\\rho c_p \\frac{\\partial T}{\\partial t}$ of "
      "the heat equation for quasi-constant specific heat $c_p$ and the density $\\rho$.");
  return params;
}

INSADHeatConductionTimeDerivative::INSADHeatConductionTimeDerivative(
    const InputParameters & parameters)
  : ADTimeDerivative(parameters),
    _temperature_td_strong_residual(getADMaterialProperty<Real>("temperature_td_strong_residual"))
{
}

ADReal
INSADHeatConductionTimeDerivative::precomputeQpResidual()
{
  return _temperature_td_strong_residual[_qp];
}
