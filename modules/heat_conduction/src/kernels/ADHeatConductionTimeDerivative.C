//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.htmlo
#include "ADHeatConductionTimeDerivative.h"

registerADMooseObject("HeatConductionApp", ADHeatConductionTimeDerivative);

defineADValidParams(
    ADHeatConductionTimeDerivative,
    ADTimeDerivative,
    params.addClassDescription(
        "AD Time derivative term $\\rho c_p \\frac{\\partial T}{\\partial t}$ of "
        "the heat equation for quasi-constant specific heat $c_p$ and the density $\\rho$.");
    params.set<bool>("use_displaced_mesh") = true;
    params.addParam<MaterialPropertyName>("specific_heat",
                                          "specific_heat",
                                          "Property name of the specific heat material property");
    params.addParam<MaterialPropertyName>("density_name",
                                          "density",
                                          "Property name of the density material property"););

template <ComputeStage compute_stage>
ADHeatConductionTimeDerivative<compute_stage>::ADHeatConductionTimeDerivative(
    const InputParameters & parameters)
  : ADTimeDerivative<compute_stage>(parameters),
    _specific_heat(adGetADMaterialProperty<Real>("specific_heat")),
    _density(adGetADMaterialProperty<Real>("density_name"))
{
}

template <ComputeStage compute_stage>
ADResidual
ADHeatConductionTimeDerivative<compute_stage>::precomputeQpResidual()
{
  return _specific_heat[_qp] * _density[_qp] *
         ADTimeDerivative<compute_stage>::precomputeQpResidual();
}
