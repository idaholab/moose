//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionTimeDerivative.h"

registerMooseObject("HeatTransferApp", HeatConductionTimeDerivative);

InputParameters
HeatConductionTimeDerivative::validParams()
{
  InputParameters params = TimeDerivative::validParams();
  params.addClassDescription("Time derivative term $\\rho c_p \\frac{\\partial T}{\\partial t}$ of "
                             "the thermal energy conservation equation.");

  // Density may be changing with deformation, so we must integrate
  // over current volume by setting the use_displaced_mesh flag.
  params.set<bool>("use_displaced_mesh") = true;

  params.addParam<MaterialPropertyName>(
      "specific_heat",
      "specific_heat",
      "Name of the volumetric isobaric specific heat material property");
  params.addParam<MaterialPropertyName>(
      "specific_heat_dT",
      "Name of the material property for the derivative of the specific heat with respect "
      "to the variable.");

  /**
   * We would like to rename this input parameter to 'density' but gratuitous use of
   * 'density' in the GlobalParams block of many many Bison simulations (for the
   * Density kernel)prevent us from doing this.
   */
  params.addParam<MaterialPropertyName>(
      "density_name", "density", "Property name of the density material property");
  params.addParam<MaterialPropertyName>(
      "density_name_dT",
      "Name of material property for the derivative of the density with respect to the variable.");
  return params;
}

HeatConductionTimeDerivative::HeatConductionTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters),
    _specific_heat(getMaterialProperty<Real>("specific_heat")),
    _specific_heat_dT(isParamValid("specific_heat_dT")
                          ? &getMaterialProperty<Real>("specific_heat_dT")
                          : nullptr),
    _density(getMaterialProperty<Real>("density_name")),
    _density_dT(isParamValid("density_name_dT") ? &getMaterialProperty<Real>("density_name_dT")
                                                : nullptr)
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
  auto jac = _specific_heat[_qp] * _density[_qp] * TimeDerivative::computeQpJacobian();
  if (_specific_heat_dT)
    jac += (*_specific_heat_dT)[_qp] * _density[_qp] * _phi[_j][_qp] *
           TimeDerivative::computeQpResidual();
  if (_density_dT)
    jac += _specific_heat[_qp] * (*_density_dT)[_qp] * _phi[_j][_qp] *
           TimeDerivative::computeQpResidual();
  return jac;
}
