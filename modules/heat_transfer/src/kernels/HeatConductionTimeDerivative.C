//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionTimeDerivative.h"

registerMooseObject("HeatConductionApp", HeatConductionTimeDerivative);

InputParameters
HeatConductionTimeDerivative::validParams()
{
  InputParameters params = TimeDerivative::validParams();
  params.addClassDescription(
      "Time derivative term $\\rho c_p \\frac{\\partial T}{\\partial t}$ of "
      "the heat equation for quasi-constant specific heat $c_p$ and the density $\\rho$.");

  // Density may be changing with deformation, so we must integrate
  // over current volume by setting the use_displaced_mesh flag.
  params.set<bool>("use_displaced_mesh") = true;

  params.addParam<MaterialPropertyName>(
      "specific_heat", "specific_heat", "Property name of the specific heat material property");

  /**
   * We would like to rename this input parameter to 'density' but gratuitous use of
   * 'density' in the GlobalParams block of many many Bison simulations (for the
   * Density kernel)prevent us from doing this.
   */
  params.addParam<MaterialPropertyName>(
      "density_name", "density", "Property name of the density material property");
  return params;
}

HeatConductionTimeDerivative::HeatConductionTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters),
    _specific_heat(getMaterialProperty<Real>("specific_heat")),
    _density(getMaterialProperty<Real>("density_name"))
{
}

Real
HeatConductionTimeDerivative::computeQpResidual()
{
  return _specific_heat[_qp] * _density[_qp] * TimeDerivative::computeQpResidual();
}

Real
HeatConductionTimeDerivative::computeQpJacobian()
{
  return _specific_heat[_qp] * _density[_qp] * TimeDerivative::computeQpJacobian();
}
