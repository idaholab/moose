//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConsistentSpecificHeatTimeDerivative.h"

template <>
InputParameters
validParams<ConsistentSpecificHeatTimeDerivative>()
{
  InputParameters params = validParams<SpecificHeatConductionTimeDerivative>();
  params.addClassDescription(
      "Time derivative term $(a1 + a2 + a3) \\frac{\\partial T}{\\partial t}$ with"
      "$a1 = c_p  \\rho$"
      "$a2 = c_p  T \\frac{\\partial \\rho}{\\partial T}$"
      "$a3 = \\rho  T \\frac{\\partial c_p}{\\partial T}$"
      " of the heat equation with the specific heat capacity $c_p$ and density $\\rho$ as "
      "arguments.");
  return params;
}

ConsistentSpecificHeatTimeDerivative::ConsistentSpecificHeatTimeDerivative(
    const InputParameters & parameters)
  : SpecificHeatConductionTimeDerivative(parameters)
{
}

Real
ConsistentSpecificHeatTimeDerivative::computeQpResidual()
{
  return (_specific_heat[_qp] * _density[_qp] + _d_density_dT[_qp] * _specific_heat[_qp] * _u[_qp] +
          _d_specific_heat_dT[_qp] * _density[_qp] * _u[_qp]) *
         TimeDerivative::computeQpResidual();
}
