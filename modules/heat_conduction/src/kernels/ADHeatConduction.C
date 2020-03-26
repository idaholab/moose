//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHeatConduction.h"

registerMooseObject("HeatConductionApp", ADHeatConduction);

InputParameters
ADHeatConduction::validParams()
{
  InputParameters params = ADDiffusion::validParams();
  params.addParam<MaterialPropertyName>("thermal_conductivity",
                                        "thermal_conductivity",
                                        "the name of the thermal conductivity material property");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

ADHeatConduction::ADHeatConduction(const InputParameters & parameters)
  : ADDiffusion(parameters),
    _thermal_conductivity(getADMaterialProperty<Real>("thermal_conductivity"))
{
}

ADRealVectorValue
ADHeatConduction::precomputeQpResidual()
{
  return _thermal_conductivity[_qp] * ADDiffusion::precomputeQpResidual();
}
