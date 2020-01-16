//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalVariableHorizonMaterialBPD.h"

registerMooseObject("PeridynamicsApp", ThermalVariableHorizonMaterialBPD);

template <>
InputParameters
validParams<ThermalVariableHorizonMaterialBPD>()
{
  InputParameters params = validParams<ThermalMaterialBaseBPD>();
  params.addClassDescription("Class for computing peridynamic micro conductivity for bond-based "
                             "model using irregular mesh");

  return params;
}

ThermalVariableHorizonMaterialBPD::ThermalVariableHorizonMaterialBPD(
    const InputParameters & parameters)
  : ThermalMaterialBaseBPD(parameters)
{
}

void
ThermalVariableHorizonMaterialBPD::computePeridynamicsParams(const Real ave_thermal_conductivity)
{
  _Kij = _dim * ave_thermal_conductivity * (1.0 / _horiz_vol[0] + 1.0 / _horiz_vol[1]) /
         _origin_length;
}
